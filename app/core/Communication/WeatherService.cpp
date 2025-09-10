#include "WeatherService.h"
#include <HTTPClient.h>
#include "core/Utils/SpiAllocator.h"

namespace Communication {

const char* WeatherService::CACHE_FILE_PATH = "/cache/weather_cache.json";

WeatherService::WeatherService(Utils::FileManager* fileManager) 
    : _lastCacheTime(0), _initialized(false), _fileManager(fileManager),
    _tag("WeatherService") {
}

WeatherService::~WeatherService() {
    // Clean up resources if needed
}

bool WeatherService::init(const WeatherConfig& config) {
    if (!_fileManager) {
        return false;
    }

    _config = config;
    _initialized = true;
    
    // Try to load existing cache
    loadCache();
    
    return true;
}

void WeatherService::getCurrentWeather(WeatherCallback callback, bool forceRefresh) {
    if (!_initialized) {
        if (callback) {
            WeatherData errorData;
            callback(errorData, false);
        }
        return;
    }

    // Check if we should use cache
    if (!forceRefresh && isCacheValid()) {
        if (callback) {
            callback(_cachedData, true);
        }
        return;
    }

    // Fetch from API
    fetchFromAPI(callback);
}

void WeatherService::setLocation(const Utils::Sstring& adm4Code) {
    _config.adm4Code = adm4Code;
    
    // Clear cache when location changes
    clearCache();
}

void WeatherService::setLocation(const IModel::AdministrativeRegion& region) {
    _config.adm4Code = region.getAdm4();
    
    // Clear cache when location changes
    clearCache();
}

void WeatherService::setCacheExpiry(uint32_t minutes) {
    _config.cacheExpiryMinutes = minutes;
}

void WeatherService::clearCache() {
    _cachedData = WeatherData();
    _lastCacheTime = 0;
    
    // Remove cache file
    if (_fileManager && _fileManager->exists(CACHE_FILE_PATH)) {
        _fileManager->deleteFile(CACHE_FILE_PATH);
    }
}

bool WeatherService::isCacheValid() const {
    if (_lastCacheTime == 0 || !_cachedData.isValid) {
        return false;
    }
    
    unsigned long currentTime = getCurrentTimestamp();
    unsigned long cacheExpiryMs = _config.cacheExpiryMinutes * 60 * 1000;
    
    return (currentTime - _lastCacheTime) < cacheExpiryMs;
}

void WeatherService::fetchFromAPI(WeatherCallback callback) {
    HTTPClient http;
    Utils::Sstring url = buildAPIUrl();
    
    http.begin(url.toString());
    http.setReuse(true);
    http.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
    http.setTimeout(10000); // 10 second timeout
    
    int httpCode = http.GET();
    
    if (httpCode == HTTP_CODE_OK) {
        Utils::Sstring response = http.getString();
        processAPIResponse(response, callback);
    } else {
        ESP_LOGE(_tag, "Error: [%d] %s", httpCode, http.errorToString(httpCode));
        if (callback) {
            WeatherData errorData;
            callback(errorData, false);
        }
    }
    
    http.end();
}

void WeatherService::processAPIResponse(const Utils::Sstring& response, WeatherCallback callback) {
    if (!callback) {
        ESP_LOGW(_tag, "No callback provided for API response");
        return;
    }

    ESP_LOGI(_tag, "Processing API response");

    Utils::SpiJsonDocument doc;
    DeserializationError error = deserializeJson(doc, response.c_str());
    
    if (error) {
        ESP_LOGE(_tag, "JSON parsing failed: %s. Value: %s", error.c_str(), response.c_str());
        WeatherData errorData;
        callback(errorData, false);
        return;
    }

    WeatherData data;
    
    try {
        // Navigate through new BMKG API structure
        // New BMKG structure: {"lokasi": {...}, "data": [{...}]}
        if (doc["lokasi"].isUnbound()) {
            ESP_LOGE(_tag, "No lokasi found in response");
            callback(data, false);
            return;
        }

        auto lokasi = doc["lokasi"];
        data.location = String(lokasi["provinsi"].as<String>()) + ", " + 
                       String(lokasi["kotkab"].as<String>()) + ", " + 
                       String(lokasi["kecamatan"].as<String>()) + ", " + 
                       String(lokasi["desa"].as<String>());
        data.longitude = lokasi["lon"].as<float>();
        data.latitude = lokasi["lat"].as<float>();
        data.timezone = lokasi["timezone"].as<String>();
        
        ESP_LOGI(_tag, "Location: %s (Lat: %.6f, Lon: %.6f)", data.location.c_str(), data.latitude, data.longitude);

        if (doc["data"].isUnbound() || doc["data"].size() == 0) {
            ESP_LOGE(_tag, "No data array found in response");
            callback(data, false);
            return;
        }

        auto dataArray = doc["data"];
        if (dataArray.size() == 0) {
            ESP_LOGE(_tag, "Empty data array in response");
            callback(data, false);
            return;
        }

        // Get the first location data
        auto locationData = dataArray[0];
        if (locationData["cuaca"].isUnbound() || locationData["cuaca"].size() == 0) {
            ESP_LOGE(_tag, "No cuaca (weather) data found");
            callback(data, false);
            return;
        }

        // Get current weather (first entry in the first time period)
        auto cuacaArray = locationData["cuaca"];
        if (cuacaArray.size() == 0 || cuacaArray[0].size() == 0) {
            ESP_LOGE(_tag, "No current weather data found");
            callback(data, false);
            return;
        }

        auto currentWeather = cuacaArray[0][0]; // First time period, first entry
        
        // Parse weather data from new API format
        data.temperature = currentWeather["t"].as<int>();
        data.humidity = currentWeather["hu"].as<int>();
        data.windSpeed = (int)(currentWeather["ws"].as<float>() * 3.6); // Convert m/s to km/h
        data.windDirection = currentWeather["wd"].as<String>();
        data.description = currentWeather["weather_desc"].as<String>();
        data.imageUrl = currentWeather["image"].as<String>();
        data.lastUpdated = currentWeather["local_datetime"].as<String>();
        
        // Convert weather code to condition
        int weatherCode = currentWeather["weather"].as<int>();
        data.condition = getConditionFromCode(weatherCode);
        
        ESP_LOGI(_tag, "Weather: %s (Code: %d)", data.description.c_str(), weatherCode);
        ESP_LOGI(_tag, "Temperature: %d°C, Humidity: %d%%, Wind: %d km/h %s", 
                 data.temperature, data.humidity, data.windSpeed, data.windDirection.c_str());

        data.isValid = true;
        
        ESP_LOGI(_tag, "Weather data parsed successfully for %s", data.location.c_str());
        
        // Cache the data
        _cachedData = data;
        _lastCacheTime = getCurrentTimestamp();
        if (saveCache(data)) {
            ESP_LOGD(_tag, "Weather data cached successfully");
        } else {
            ESP_LOGW(_tag, "Failed to cache weather data");
        }
        
        callback(data, true);
        
    } catch (...) {
        ESP_LOGE(_tag, "Exception occurred while processing API response");
        WeatherData errorData;
        callback(errorData, false);
    }
}

bool WeatherService::loadCache() {
    if (!_fileManager || !_fileManager->exists(CACHE_FILE_PATH)) {
        return false;
    }

    Utils::Sstring content = _fileManager->readFile(CACHE_FILE_PATH);
    if (content.isEmpty()) {
        return false;
    }

    Utils::SpiJsonDocument doc;
    DeserializationError error = deserializeJson(doc, content.c_str());
    
    if (error) {
        return false;
    }

    _cachedData.location = doc["location"].as<String>();
    _cachedData.description = doc["description"].as<String>();
    _cachedData.condition = static_cast<WeatherCondition>(doc["condition"].as<int>());
    _cachedData.temperature = doc["temperature"].as<int>();
    _cachedData.humidity = doc["humidity"].as<int>();
    _cachedData.windSpeed = doc["windSpeed"].as<int>();
    _cachedData.windDirection = doc["windDirection"].as<String>();
    _cachedData.lastUpdated = doc["lastUpdated"].as<String>();
    _cachedData.imageUrl = doc["imageUrl"].as<String>();
    _cachedData.longitude = doc["longitude"].as<float>();
    _cachedData.latitude = doc["latitude"].as<float>();
    _cachedData.timezone = doc["timezone"].as<String>();
    _cachedData.isValid = doc["isValid"].as<bool>();
    _lastCacheTime = doc["cacheTime"].as<unsigned long>();

    return true;
}

bool WeatherService::saveCache(const WeatherData& data) {
    if (!_fileManager) {
        return false;
    }

    Utils::SpiJsonDocument doc;
    
    doc["location"] = data.location;
    doc["description"] = data.description;
    doc["condition"] = static_cast<int>(data.condition);
    doc["temperature"] = data.temperature;
    doc["humidity"] = data.humidity;
    doc["windSpeed"] = data.windSpeed;
    doc["windDirection"] = data.windDirection;
    doc["lastUpdated"] = data.lastUpdated;
    doc["imageUrl"] = data.imageUrl;
    doc["longitude"] = data.longitude;
    doc["latitude"] = data.latitude;
    doc["timezone"] = data.timezone;
    doc["isValid"] = data.isValid;
    doc["cacheTime"] = getCurrentTimestamp();

    String jsonString;
    serializeJson(doc, jsonString);

    return _fileManager->writeFile(CACHE_FILE_PATH, jsonString.c_str());
}

Utils::Sstring WeatherService::buildAPIUrl() const {
    // New BMKG API endpoint format
    Utils::Sstring url = "https://api.bmkg.go.id/publik/prakiraan-cuaca?adm4=";
    url += _config.adm4Code;
    
    return url;
}

Utils::Sstring WeatherService::parseBMKGDateTime(const Utils::Sstring& bmkgDateTime) const {
    // BMKG usually returns ISO format like: "2025-09-05T14:30:00"
    // We'll return a simplified format
    Utils::Sstring result = bmkgDateTime;
    result.replace("T", " ");
    
    // Remove timezone info if present
    int dotPos = result.indexOf(".");
    if (dotPos != -1) {
        result = result.substring(0, dotPos);
    }
    
    return result;
}

unsigned long WeatherService::getCurrentTimestamp() const {
    return millis();
}

WeatherService::WeatherParam WeatherService::getParamFromString(const Utils::Sstring& paramId) {
    if (paramId == "weather") return WeatherParam::WEATHER;
    if (paramId == "t") return WeatherParam::TEMPERATURE;
    if (paramId == "hu") return WeatherParam::HUMIDITY;
    if (paramId == "ws") return WeatherParam::WIND_SPEED;
    if (paramId == "wd") return WeatherParam::WIND_DIRECTION;
    if (paramId == "p") return WeatherParam::PRESSURE;
    if (paramId == "vs") return WeatherParam::VISIBILITY;
    if (paramId == "uv") return WeatherParam::UV_INDEX;
    return WeatherParam::UNKNOWN;
}

WeatherService::WeatherCondition WeatherService::getConditionFromDescription(const Utils::Sstring& description) {
    // Convert to String for easier manipulation
    String desc = description.toString();
    desc.toLowerCase();
    
    if (desc.indexOf("cerah") >= 0 || desc.indexOf("clear") >= 0 || desc.indexOf("sunny") >= 0) {
        return WeatherCondition::CLEAR;
    }
    if (desc.indexOf("cerah berawan") >= 0 || desc.indexOf("partly cloudy") >= 0) {
        return WeatherCondition::PARTLY_CLOUDY;
    }
    if (desc.indexOf("berawan") >= 0 || desc.indexOf("cloudy") >= 0 || desc.indexOf("mostly cloudy") >= 0) {
        return WeatherCondition::CLOUDY;
    }
    if (desc.indexOf("mendung") >= 0 || desc.indexOf("overcast") >= 0) {
        return WeatherCondition::OVERCAST;
    }
    if (desc.indexOf("hujan ringan") >= 0 || desc.indexOf("light rain") >= 0) {
        return WeatherCondition::LIGHT_RAIN;
    }
    if (desc.indexOf("hujan sedang") >= 0 || desc.indexOf("moderate rain") >= 0) {
        return WeatherCondition::MODERATE_RAIN;
    }
    if (desc.indexOf("hujan lebat") >= 0 || desc.indexOf("heavy rain") >= 0) {
        return WeatherCondition::HEAVY_RAIN;
    }
    if (desc.indexOf("petir") >= 0 || desc.indexOf("thunder") >= 0) {
        return WeatherCondition::THUNDERSTORM;
    }
    if (desc.indexOf("kabut") >= 0 || desc.indexOf("fog") >= 0) {
        return WeatherCondition::FOG;
    }
    if (desc.indexOf("berkabut") >= 0 || desc.indexOf("mist") >= 0) {
        return WeatherCondition::MIST;
    }
    
    return WeatherCondition::UNKNOWN;
}

WeatherService::WeatherCondition WeatherService::getConditionFromCode(int weatherCode) {
    // BMKG weather codes:
    // 0 = Cerah (Clear/Sunny)
    // 1 = Cerah Berawan (Partly Cloudy)
    // 2 = Cerah Berawan (Partly Cloudy)
    // 3 = Berawan (Cloudy/Mostly Cloudy)
    // 4 = Berawan Tebal (Overcast)
    // And other codes for rain, thunderstorm, etc.
    
    switch (weatherCode) {
        case 0:
            return WeatherCondition::CLEAR;
        case 1:
        case 2:
            return WeatherCondition::PARTLY_CLOUDY;
        case 3:
            return WeatherCondition::CLOUDY;
        case 4:
            return WeatherCondition::OVERCAST;
        case 60:
        case 61:
            return WeatherCondition::LIGHT_RAIN;
        case 63:
            return WeatherCondition::MODERATE_RAIN;
        case 65:
            return WeatherCondition::HEAVY_RAIN;
        case 95:
        case 97:
            return WeatherCondition::THUNDERSTORM;
        case 45:
        case 48:
            return WeatherCondition::FOG;
        default:
            return WeatherCondition::UNKNOWN;
    }
}

IModel::AdministrativeRegion* WeatherService::getCurrentRegion() const {
    if (_config.adm4Code.isEmpty()) {
        return nullptr;
    }
    
    return IModel::AdministrativeRegion::findByAdm4(_config.adm4Code.c_str());
}

Utils::Sstring WeatherService::paramToString(WeatherParam param) {
    switch (param) {
        case WeatherParam::WEATHER: return "weather";
        case WeatherParam::TEMPERATURE: return "temperature";
        case WeatherParam::HUMIDITY: return "humidity";
        case WeatherParam::WIND_SPEED: return "wind_speed";
        case WeatherParam::WIND_DIRECTION: return "wind_direction";
        case WeatherParam::PRESSURE: return "pressure";
        case WeatherParam::VISIBILITY: return "visibility";
        case WeatherParam::UV_INDEX: return "uv_index";
        default: return "unknown";
    }
}

Utils::Sstring WeatherService::conditionToString(WeatherCondition condition) {
    switch (condition) {
        case WeatherCondition::CLEAR: return "Clear";
        case WeatherCondition::PARTLY_CLOUDY: return "Partly Cloudy";
        case WeatherCondition::CLOUDY: return "Cloudy";
        case WeatherCondition::OVERCAST: return "Overcast";
        case WeatherCondition::LIGHT_RAIN: return "Light Rain";
        case WeatherCondition::MODERATE_RAIN: return "Moderate Rain";
        case WeatherCondition::HEAVY_RAIN: return "Heavy Rain";
        case WeatherCondition::THUNDERSTORM: return "Thunderstorm";
        case WeatherCondition::FOG: return "Fog";
        case WeatherCondition::MIST: return "Mist";
        default: return "Unknown";
    }
}

Utils::Sstring WeatherService::getProvinceName(Province province) {
    switch (province) {
        case Province::ACEH: return "Aceh";
        case Province::SUMATERA_UTARA: return "Sumatera Utara";
        case Province::SUMATERA_BARAT: return "Sumatera Barat";
        case Province::RIAU: return "Riau";
        case Province::JAMBI: return "Jambi";
        case Province::SUMATERA_SELATAN: return "Sumatera Selatan";
        case Province::BENGKULU: return "Bengkulu";
        case Province::LAMPUNG: return "Lampung";
        case Province::KEP_BANGKA_BELITUNG: return "Kepulauan Bangka Belitung";
        case Province::KEP_RIAU: return "Kepulauan Riau";
        case Province::DKI_JAKARTA: return "DKI Jakarta";
        case Province::JAWA_BARAT: return "Jawa Barat";
        case Province::JAWA_TENGAH: return "Jawa Tengah";
        case Province::DI_YOGYAKARTA: return "DI Yogyakarta";
        case Province::JAWA_TIMUR: return "Jawa Timur";
        case Province::BANTEN: return "Banten";
        case Province::BALI: return "Bali";
        case Province::NUSA_TENGGARA_BARAT: return "Nusa Tenggara Barat";
        case Province::NUSA_TENGGARA_TIMUR: return "Nusa Tenggara Timur";
        case Province::KALIMANTAN_BARAT: return "Kalimantan Barat";
        case Province::KALIMANTAN_TENGAH: return "Kalimantan Tengah";
        case Province::KALIMANTAN_SELATAN: return "Kalimantan Selatan";
        case Province::KALIMANTAN_TIMUR: return "Kalimantan Timur";
        case Province::KALIMANTAN_UTARA: return "Kalimantan Utara";
        case Province::SULAWESI_UTARA: return "Sulawesi Utara";
        case Province::SULAWESI_TENGAH: return "Sulawesi Tengah";
        case Province::SULAWESI_SELATAN: return "Sulawesi Selatan";
        case Province::SULAWESI_TENGGARA: return "Sulawesi Tenggara";
        case Province::GORONTALO: return "Gorontalo";
        case Province::SULAWESI_BARAT: return "Sulawesi Barat";
        case Province::MALUKU: return "Maluku";
        case Province::MALUKU_UTARA: return "Maluku Utara";
        case Province::PAPUA_BARAT: return "Papua Barat";
        case Province::PAPUA: return "Papua";
        default: return "Unknown Province";
    }
}

} // namespace Communication

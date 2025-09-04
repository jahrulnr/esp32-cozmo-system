#include "WeatherService.h"
#include <HTTPClient.h>
#include "core/Utils/SpiAllocator.h"

namespace Communication {

const char* WeatherService::CACHE_FILE_PATH = "/data/weather_cache.json";

WeatherService::WeatherService(Utils::FileManager* fileManager) 
    : _lastCacheTime(0), _initialized(false), _fileManager(fileManager) {
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

void WeatherService::setLocation(Province province, int cityCode) {
    _config.province = province;
    _config.cityCode = cityCode;
    
    // Clear cache when location changes
    clearCache();
}

void WeatherService::setJakartaLocation(JakartaCity city) {
    setLocation(Province::DKI_JAKARTA, static_cast<int>(city));
}

void WeatherService::setJawaBaratLocation(JawaBaratCity city) {
    setLocation(Province::JAWA_BARAT, static_cast<int>(city));
}

void WeatherService::setJawaTengahLocation(JawaTengahCity city) {
    setLocation(Province::JAWA_TENGAH, static_cast<int>(city));
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
    http.addHeader("User-Agent", "ESP32-Cozmo-Weather/1.0");
    http.setReuse(true);
    http.setTimeout(10000); // 10 second timeout
    
    int httpCode = http.GET();
    
    if (httpCode == HTTP_CODE_OK) {
        Utils::Sstring response = http.getString();
        processAPIResponse(response, callback);
    } else {
        if (callback) {
            WeatherData errorData;
            callback(errorData, false);
        }
    }
    
    http.end();
}

void WeatherService::processAPIResponse(const Utils::Sstring& response, WeatherCallback callback) {
    if (!callback) {
        return;
    }

    Utils::SpiJsonDocument doc;
    DeserializationError error = deserializeJson(doc, response.c_str());
    
    if (error) {
        WeatherData errorData;
        callback(errorData, false);
        return;
    }

    WeatherData data;
    
    try {
        // Navigate through BMKG JSON structure
        // BMKG structure: {"data": [{"areas": [{"params": [...]}]}]}
        if (doc["data"].isUnbound() || doc["data"].size() == 0) {
            callback(data, false);
            return;
        }

        auto dataArray = doc["data"];
        if (dataArray.size() == 0) {
            callback(data, false);
            return;
        }

        auto areas = dataArray[0]["areas"];
        if (areas.isUnbound() || areas.size() == 0) {
            callback(data, false);
            return;
        }

        // Find the area that matches our city code
        bool foundArea = false;
        for (size_t i = 0; i < areas.size(); i++) {
            auto area = areas[i];
            int areaId = area["id"].as<int>();
            if (areaId == _config.cityCode) {
                foundArea = true;
                data.location = area["description"].as<String>();
                
                // Parse weather parameters using enum mapping
                auto params = area["params"];
                for (size_t j = 0; j < params.size(); j++) {
                    auto param = params[j];
                    Utils::Sstring paramId = param["id"].as<String>();
                    WeatherParam paramType = getParamFromString(paramId);
                    
                    auto timeRanges = param["timeRanges"];
                    if (timeRanges.size() == 0) continue;
                    
                    auto firstRange = timeRanges[0];
                    
                    switch (paramType) {
                        case WeatherParam::WEATHER:
                            data.description = firstRange["value"]["text"].as<String>();
                            data.condition = getConditionFromDescription(data.description);
                            break;
                        case WeatherParam::TEMPERATURE:
                            data.temperature = firstRange["value"].as<int>();
                            break;
                        case WeatherParam::HUMIDITY:
                            data.humidity = firstRange["value"].as<int>();
                            break;
                        case WeatherParam::WIND_SPEED:
                            data.windSpeed = firstRange["value"].as<int>();
                            break;
                        case WeatherParam::WIND_DIRECTION:
                            data.windDirection = firstRange["value"]["text"].as<String>();
                            break;
                        default:
                            // Ignore unknown parameters
                            break;
                    }
                }
                break;
            }
        }

        if (!foundArea) {
            callback(data, false);
            return;
        }

        // Set timestamp and validity
        data.lastUpdated = parseBMKGDateTime(dataArray[0]["issued"].as<String>());
        data.isValid = true;
        
        // Cache the data
        _cachedData = data;
        _lastCacheTime = getCurrentTimestamp();
        saveCache(data);
        
        callback(data, true);
        
    } catch (...) {
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
    doc["isValid"] = data.isValid;
    doc["cacheTime"] = getCurrentTimestamp();

    String jsonString;
    serializeJson(doc, jsonString);

    return _fileManager->writeFile(CACHE_FILE_PATH, jsonString.c_str());
}

Utils::Sstring WeatherService::buildAPIUrl() const {
    // BMKG API endpoint format
    Utils::Sstring url = "https://data.bmkg.go.id/DataMKG/MEWS/DigitalForecast/DigitalForecast-";
    url += String(static_cast<int>(_config.province));
    url += ".xml";
    
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
    
    if (desc.indexOf("cerah") >= 0 || desc.indexOf("clear") >= 0) {
        return WeatherCondition::CLEAR;
    }
    if (desc.indexOf("berawan sebagian") >= 0 || desc.indexOf("partly cloudy") >= 0) {
        return WeatherCondition::PARTLY_CLOUDY;
    }
    if (desc.indexOf("berawan") >= 0 || desc.indexOf("cloudy") >= 0) {
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

Utils::Sstring WeatherService::getJakartaCityName(JakartaCity city) {
    switch (city) {
        case JakartaCity::JAKARTA_PUSAT: return "Jakarta Pusat";
        case JakartaCity::JAKARTA_UTARA: return "Jakarta Utara";
        case JakartaCity::JAKARTA_BARAT: return "Jakarta Barat";
        case JakartaCity::JAKARTA_SELATAN: return "Jakarta Selatan";
        case JakartaCity::JAKARTA_TIMUR: return "Jakarta Timur";
        case JakartaCity::KEP_SERIBU: return "Kepulauan Seribu";
        default: return "Unknown Jakarta City";
    }
}

Utils::Sstring WeatherService::getJawaBaratCityName(JawaBaratCity city) {
    switch (city) {
        case JawaBaratCity::BOGOR: return "Bogor";
        case JawaBaratCity::SUKABUMI: return "Sukabumi";
        case JawaBaratCity::CIANJUR: return "Cianjur";
        case JawaBaratCity::BANDUNG: return "Bandung";
        case JawaBaratCity::GARUT: return "Garut";
        case JawaBaratCity::TASIKMALAYA: return "Tasikmalaya";
        case JawaBaratCity::CIAMIS: return "Ciamis";
        case JawaBaratCity::KUNINGAN: return "Kuningan";
        case JawaBaratCity::CIREBON: return "Cirebon";
        case JawaBaratCity::MAJALENGKA: return "Majalengka";
        case JawaBaratCity::SUMEDANG: return "Sumedang";
        case JawaBaratCity::INDRAMAYU: return "Indramayu";
        case JawaBaratCity::SUBANG: return "Subang";
        case JawaBaratCity::PURWAKARTA: return "Purwakarta";
        case JawaBaratCity::KARAWANG: return "Karawang";
        case JawaBaratCity::BEKASI: return "Bekasi";
        case JawaBaratCity::BANDUNG_BARAT: return "Bandung Barat";
        case JawaBaratCity::PANGANDARAN: return "Pangandaran";
        default: return "Unknown Jawa Barat City";
    }
}

Utils::Sstring WeatherService::getJawaTengahCityName(JawaTengahCity city) {
    switch (city) {
        case JawaTengahCity::CILACAP: return "Cilacap";
        case JawaTengahCity::BANYUMAS: return "Banyumas";
        case JawaTengahCity::PURBALINGGA: return "Purbalingga";
        case JawaTengahCity::BANJARNEGARA: return "Banjarnegara";
        case JawaTengahCity::KEBUMEN: return "Kebumen";
        case JawaTengahCity::PURWOREJO: return "Purworejo";
        case JawaTengahCity::WONOSOBO: return "Wonosobo";
        case JawaTengahCity::MAGELANG: return "Magelang";
        case JawaTengahCity::BOYOLALI: return "Boyolali";
        case JawaTengahCity::KLATEN: return "Klaten";
        case JawaTengahCity::SUKOHARJO: return "Sukoharjo";
        case JawaTengahCity::WONOGIRI: return "Wonogiri";
        case JawaTengahCity::KARANGANYAR: return "Karanganyar";
        case JawaTengahCity::SRAGEN: return "Sragen";
        case JawaTengahCity::GROBOGAN: return "Grobogan";
        case JawaTengahCity::BLORA: return "Blora";
        case JawaTengahCity::REMBANG: return "Rembang";
        case JawaTengahCity::PATI: return "Pati";
        case JawaTengahCity::KUDUS: return "Kudus";
        case JawaTengahCity::JEPARA: return "Jepara";
        case JawaTengahCity::DEMAK: return "Demak";
        case JawaTengahCity::SEMARANG: return "Semarang";
        case JawaTengahCity::TEMANGGUNG: return "Temanggung";
        case JawaTengahCity::KENDAL: return "Kendal";
        case JawaTengahCity::BATANG: return "Batang";
        case JawaTengahCity::PEKALONGAN: return "Pekalongan";
        case JawaTengahCity::PEMALANG: return "Pemalang";
        case JawaTengahCity::TEGAL: return "Tegal";
        case JawaTengahCity::BREBES: return "Brebes";
        default: return "Unknown Jawa Tengah City";
    }
}

} // namespace Communication

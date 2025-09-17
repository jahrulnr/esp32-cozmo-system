#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <functional>
#include "Sstring.h"
#include "FileManager.h"
#include "repository/AdministrativeRegion.h"

namespace Services {

class WeatherService {
public:
    // Indonesian Provinces (BMKG codes)
    enum class Province {
        ACEH = 11,
        SUMATERA_UTARA = 12,
        SUMATERA_BARAT = 13,
        RIAU = 14,
        JAMBI = 15,
        SUMATERA_SELATAN = 16,
        BENGKULU = 17,
        LAMPUNG = 18,
        KEP_BANGKA_BELITUNG = 19,
        KEP_RIAU = 21,
        DKI_JAKARTA = 31,
        JAWA_BARAT = 32,
        JAWA_TENGAH = 33,
        DI_YOGYAKARTA = 34,
        JAWA_TIMUR = 35,
        BANTEN = 36,
        BALI = 51,
        NUSA_TENGGARA_BARAT = 52,
        NUSA_TENGGARA_TIMUR = 53,
        KALIMANTAN_BARAT = 61,
        KALIMANTAN_TENGAH = 62,
        KALIMANTAN_SELATAN = 63,
        KALIMANTAN_TIMUR = 64,
        KALIMANTAN_UTARA = 65,
        SULAWESI_UTARA = 71,
        SULAWESI_TENGAH = 72,
        SULAWESI_SELATAN = 73,
        SULAWESI_TENGGARA = 74,
        GORONTALO = 75,
        SULAWESI_BARAT = 76,
        MALUKU = 81,
        MALUKU_UTARA = 82,
        PAPUA_BARAT = 91,
        PAPUA = 94
    };

    // Major Cities for DKI Jakarta (most commonly used)
    enum class JakartaCity {
        JAKARTA_PUSAT = 3171,
        JAKARTA_UTARA = 3172,
        JAKARTA_BARAT = 3173,
        JAKARTA_SELATAN = 3174,
        JAKARTA_TIMUR = 3175,
        KEP_SERIBU = 3176
    };

    // Major Cities for Jawa Barat
    enum class JawaBaratCity {
        BOGOR = 3201,
        SUKABUMI = 3202,
        CIANJUR = 3203,
        BANDUNG = 3204,
        GARUT = 3205,
        TASIKMALAYA = 3206,
        CIAMIS = 3207,
        KUNINGAN = 3208,
        CIREBON = 3209,
        MAJALENGKA = 3210,
        SUMEDANG = 3211,
        INDRAMAYU = 3212,
        SUBANG = 3213,
        PURWAKARTA = 3214,
        KARAWANG = 3215,
        BEKASI = 3216,
        BANDUNG_BARAT = 3217,
        PANGANDARAN = 3218
    };

    // Major Cities for Jawa Tengah
    enum class JawaTengahCity {
        CILACAP = 3301,
        BANYUMAS = 3302,
        PURBALINGGA = 3303,
        BANJARNEGARA = 3304,
        KEBUMEN = 3305,
        PURWOREJO = 3306,
        WONOSOBO = 3307,
        MAGELANG = 3308,
        BOYOLALI = 3309,
        KLATEN = 3310,
        SUKOHARJO = 3311,
        WONOGIRI = 3312,
        KARANGANYAR = 3313,
        SRAGEN = 3314,
        GROBOGAN = 3315,
        BLORA = 3316,
        REMBANG = 3317,
        PATI = 3318,
        KUDUS = 3319,
        JEPARA = 3320,
        DEMAK = 3321,
        SEMARANG = 3322,
        TEMANGGUNG = 3323,
        KENDAL = 3324,
        BATANG = 3325,
        PEKALONGAN = 3326,
        PEMALANG = 3327,
        TEGAL = 3328,
        BREBES = 3329
    };

    // BMKG Weather Parameter Types
    enum class WeatherParam {
        WEATHER = 0,    // weather - Weather condition description
        TEMPERATURE,    // t - Temperature in Celsius
        HUMIDITY,       // hu - Humidity percentage
        WIND_SPEED,     // ws - Wind speed in km/h
        WIND_DIRECTION, // wd - Wind direction
        PRESSURE,       // p - Air pressure
        VISIBILITY,     // vs - Visibility in km
        UV_INDEX,       // uv - UV index
        UNKNOWN
    };

    // Weather Condition Categories
    enum class WeatherCondition {
        CLEAR = 0,
        PARTLY_CLOUDY,
        CLOUDY,
        OVERCAST,
        LIGHT_RAIN,
        MODERATE_RAIN,
        HEAVY_RAIN,
        THUNDERSTORM,
        FOG,
        MIST,
        UNKNOWN
    };

    struct WeatherData {
        Utils::Sstring location;
        Utils::Sstring description;
        WeatherCondition condition;     // Categorized weather condition
        int temperature;                // in Celsius
        int humidity;                   // in percentage
        int windSpeed;                  // in km/h
        Utils::Sstring windDirection;
        Utils::Sstring lastUpdated;
        Utils::Sstring imageUrl;        // Weather icon URL
        float longitude;                // Location longitude
        float latitude;                 // Location latitude
        Utils::Sstring timezone;        // Location timezone
        bool isValid;

        WeatherData() : condition(WeatherCondition::UNKNOWN), temperature(0), humidity(0),
                       windSpeed(0), longitude(0.0), latitude(0.0), isValid(false) {}
    };

    struct WeatherConfig {
        Utils::Sstring adm4Code;        // Administrative level 4 code (village/kelurahan)
        uint32_t cacheExpiryMinutes;    // Cache expiry time in minutes

        WeatherConfig() : adm4Code("31.71.03.1001"), cacheExpiryMinutes(60) {}
    };

    // Callback type for weather responses
    using WeatherCallback = std::function<void(const WeatherData& data, bool success)>;

    WeatherService(Utils::FileManager* fileManager);
    ~WeatherService();

    /**
     * Initialize weather service
     * @param config Weather service configuration
     * @return true if initialization was successful, false otherwise
     */
    bool init(const WeatherConfig& config);

    /**
     * Check if the service is initialized
     * @return true if initialized, false otherwise
     */
    bool isInitialized() const { return _initialized; }

    /**
     * Get current weather data
     * @param callback Callback function for the weather data
     * @param forceRefresh Force refresh from API (ignore cache)
     */
    void getCurrentWeather(WeatherCallback callback, bool forceRefresh = false);

    /**
     * Set the location by administrative region code
     * @param adm4Code Administrative level 4 code (village/kelurahan)
     */
    void setLocation(const Utils::Sstring& adm4Code);

    /**
     * Set the location by administrative region
     * @param region Administrative region object
     */
    void setLocation(const IModel::AdministrativeRegion& region);

    /**
     * Set cache expiry time
     * @param minutes Cache expiry time in minutes
     */
    void setCacheExpiry(uint32_t minutes);

    /**
     * Get current configuration
     * @return Current weather configuration
     */
    WeatherConfig getConfig() const { return _config; }

    /**
     * Clear cached weather data
     */
    void clearCache();

    /**
     * Check if cached data is still valid
     * @return true if cache is valid, false otherwise
     */
    bool isCacheValid() const;

    /**
     * Get last cached weather data
     * @return Last cached weather data (may be expired)
     */
    WeatherData getCachedData() const { return _cachedData; }

    /**
     * Convert BMKG parameter ID to enum
     * @param paramId BMKG parameter ID string
     * @return WeatherParam enum value
     */
    static WeatherParam getParamFromString(const Utils::Sstring& paramId);

    /**
     * Get administrative region for current location
     * @return Pointer to AdministrativeRegion object (caller owns memory)
     */
    IModel::AdministrativeRegion* getCurrentRegion() const;

    /**
     * Convert weather description to categorized condition (updated for new API)
     * @param description Weather description string
     * @return WeatherCondition enum value
     */
    static WeatherCondition getConditionFromDescription(const Utils::Sstring& description);

    /**
     * Convert weather code to categorized condition
     * @param weatherCode BMKG weather code (0-4)
     * @return WeatherCondition enum value
     */
    static WeatherCondition getConditionFromCode(int weatherCode);

    /**
     * Get string representation of weather parameter
     * @param param WeatherParam enum value
     * @return String representation
     */
    static Utils::Sstring paramToString(WeatherParam param);

    /**
     * Get string representation of weather condition
     * @param condition WeatherCondition enum value
     * @return String representation
     */
    static Utils::Sstring conditionToString(WeatherCondition condition);

    /**
     * Get province name from enum
     * @param province Province enum value
     * @return Province name
     */
    static Utils::Sstring getProvinceName(Province province);

private:
    const char* _tag;
    WeatherConfig _config;
    WeatherData _cachedData;
    unsigned long _lastCacheTime;
    bool _initialized;
    Utils::FileManager* _fileManager;

    // Cache file path
    static const char* CACHE_FILE_PATH;

    /**
     * Fetch weather data from BMKG API
     * @param callback Callback function for the response
     */
    void fetchFromAPI(WeatherCallback callback);

    /**
     * Process the HTTP response from BMKG API
     * @param response Raw HTTP response
     * @param callback Callback function for processed data
     */
    void processAPIResponse(const Utils::Sstring& response, WeatherCallback callback);

    /**
     * Load cached weather data from file
     * @return true if cache was loaded successfully, false otherwise
     */
    bool loadCache();

    /**
     * Save weather data to cache file
     * @param data Weather data to cache
     * @return true if cache was saved successfully, false otherwise
     */
    bool saveCache(const WeatherData& data);

    /**
     * Build BMKG API URL based on current configuration
     * @return Complete API URL
     */
    Utils::Sstring buildAPIUrl() const;

    /**
     * Parse date/time from BMKG format
     * @param bmkgDateTime BMKG date/time string
     * @return Formatted date/time string
     */
    Utils::Sstring parseBMKGDateTime(const Utils::Sstring& bmkgDateTime) const;

    /**
     * Get current timestamp in milliseconds
     * @return Current timestamp
     */
    unsigned long getCurrentTimestamp() const;
};

} // namespace Communication

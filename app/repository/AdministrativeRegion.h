#ifndef ADMINISTRATIVE_REGION_MODEL_H
#define ADMINISTRATIVE_REGION_MODEL_H

#include "Database/Model.h"
#include <Arduino.h>
#include <set>
#include <vector>

namespace IModel {

class AdministrativeRegion : public Model {
public:
    // Constructor
    AdministrativeRegion() : Model("administrative_regions") {
				initTable();
		}
    
    // Create a new administrative region instance
    AdministrativeRegion(const String& adm1, const String& adm2, const String& adm3, const String& adm4,
                        const String& provinsi, const String& kotkab, const String& kecamatan, const String& desa) 
        : Model("administrative_regions") {
				initTable();
        setAdm1(adm1);
        setAdm2(adm2);
        setAdm3(adm3);
        setAdm4(adm4);
        setProvinsi(provinsi);
        setKotkab(kotkab);
        setKecamatan(kecamatan);
        setDesa(desa);
    }
    
    // Getters
    String getAdm1() const { return get("adm1"); }
    String getAdm2() const { return get("adm2"); }
    String getAdm3() const { return get("adm3"); }
    String getAdm4() const { return get("adm4"); }
    String getProvinsi() const { return get("provinsi"); }
    String getKotkab() const { return get("kotkab"); }
    String getKecamatan() const { return get("kecamatan"); }
    String getDesa() const { return get("desa"); }
    
    // Setters
    void setAdm1(const String& adm1) { set("adm1", adm1); }
    void setAdm2(const String& adm2) { set("adm2", adm2); }
    void setAdm3(const String& adm3) { set("adm3", adm3); }
    void setAdm4(const String& adm4) { set("adm4", adm4); }
    void setProvinsi(const String& provinsi) { set("provinsi", provinsi); }
    void setKotkab(const String& kotkab) { set("kotkab", kotkab); }
    void setKecamatan(const String& kecamatan) { set("kecamatan", kecamatan); }
    void setDesa(const String& desa) { set("desa", desa); }
    
    // Find administrative region by adm4 code
    static AdministrativeRegion* findByAdm4(const String& adm4) {
        CsvDatabase* db = Model::getDatabase();
        if (!db) return nullptr;
        
        // Make sure table exists
        if (!db->tableExists("administrative_regions")) {
            if (!initTable()) {
                return nullptr;
            }
        }
        
        // Find by adm4
        std::map<String, String> conditions;
        conditions["adm4"] = adm4;
        
        std::vector<std::map<String, String>> results = db->select("administrative_regions", conditions);
        
        if (results.size() > 0) {
            AdministrativeRegion* region = new AdministrativeRegion();
            region->fill(results[0]);
            return region;
        }
        
        return nullptr;
    }
    
    // Find administrative regions by province
    static std::vector<AdministrativeRegion*> findByProvinsi(const String& provinsi) {
        CsvDatabase* db = Model::getDatabase();
        std::vector<AdministrativeRegion*> regions;
        
        if (!db) return regions;
        
        // Make sure table exists
        if (!db->tableExists("administrative_regions")) {
            if (!initTable()) {
                return regions;
            }
        }
        
        // Find by province
        std::map<String, String> conditions;
        conditions["provinsi"] = provinsi;
        
        std::vector<std::map<String, String>> results = db->select("administrative_regions", conditions);
        
        for (const auto& result : results) {
            AdministrativeRegion* region = new AdministrativeRegion();
            region->fill(result);
            regions.push_back(region);
        }
        
        return regions;
    }
    
    // Find administrative regions by city/kabupaten
    static std::vector<AdministrativeRegion*> findByKotkab(const String& kotkab) {
        CsvDatabase* db = Model::getDatabase();
        std::vector<AdministrativeRegion*> regions;
        
        if (!db) return regions;
        
        // Make sure table exists
        if (!db->tableExists("administrative_regions")) {
            if (!initTable()) {
                return regions;
            }
        }
        
        // Find by kotkab
        std::map<String, String> conditions;
        conditions["kotkab"] = kotkab;
        
        std::vector<std::map<String, String>> results = db->select("administrative_regions", conditions);
        
        for (const auto& result : results) {
            AdministrativeRegion* region = new AdministrativeRegion();
            region->fill(result);
            regions.push_back(region);
        }
        
        return regions;
    }
    
    // Get all provinces
    static std::vector<String> getAllProvinces() {
        CsvDatabase* db = Model::getDatabase();
        std::vector<String> provinces;
        
        if (!db) return provinces;
        
        // Make sure table exists
        if (!db->tableExists("administrative_regions")) {
            if (!initTable()) {
                return provinces;
            }
        }
        
        std::vector<std::map<String, String>> results = db->select("administrative_regions");
        std::set<String> uniqueProvinces; // Use set to avoid duplicates
        
        for (const auto& result : results) {
            if (result.find("provinsi") != result.end()) {
                uniqueProvinces.insert(result.at("provinsi"));
            }
        }
        
        // Convert set to vector
        for (const auto& province : uniqueProvinces) {
            provinces.push_back(province);
        }
        
        return provinces;
    }
private:
    // Initialize administrative regions table
    static bool initTable() {
        // Get the database from Model
        CsvDatabase* db = Model::getDatabase();
        if (!db) return false;
        
        // Create table if it doesn't exist
        if (!db->tableExists("administrative_regions")) {
            std::vector<String> columns = {"adm1", "adm2", "adm3", "adm4", "provinsi", "kotkab", "kecamatan", "desa"};
            return db->createTable("administrative_regions", columns);
        }
        
        return true;
    }
};

}; // end namespace

#endif // ADMINISTRATIVE_REGION_MODEL_H

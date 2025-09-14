#include "include/RedisDatabase.h"
#include <fstream>
#include <sstream>
#include <mutex>
#include <algorithm>
#include <iterator>
#include <unordered_map>
#include <string>
RedisDatabase& RedisDatabase::getInstance(){
    static RedisDatabase instance;
    return instance;
}
bool RedisDatabase::flushAll(){
    std::lock_guard<std::mutex> lock(db_mutex);
    kv_store.clear();
    list_store.clear();
    hash_store.clear();
    return true;
}
void RedisDatabase::set(std::string& key, std::string& value){
    std::lock_guard<std::mutex> lock(db_mutex);
    kv_store[key]= value;
}
bool RedisDatabase::get(std::string& key, std::string& value){
    std::lock_guard<std::mutex> lock(db_mutex);
    auto it= kv_store.find(key);
    if(it!=kv_store.end()){
        value= it->second;
        return true;
    }
    return false;
}
std::vector<std::string> RedisDatabase::keys(){
    std::lock_guard<std::mutex> lock(db_mutex);
    std::vector<std::string> result;
    for(const auto& pair: kv_store){
        result.push_back(pair.first);
    }
    for(const auto& pair:list_store){
        result.push_back(pair.first);
    }
    for(const auto& pair:hash_store){
        result.push_back(pair.first);
    }
    return result;

}
std::string RedisDatabase::type(std::string& key){
    std::lock_guard<std::mutex> lock(db_mutex);
    if(kv_store.find(key)!=kv_store.end()){
        return "string";
    }else if(list_store.find(key)!=list_store.end()){
        return "list";
    }else if(hash_store.find(key)!=hash_store.end()){
        return "hash";
    }else{
        return "none";
    }
}
bool RedisDatabase::del(std::string& key){
    std::lock_guard<std::mutex> lock(db_mutex);
    bool erased= false;
    erased!= (kv_store.erase(key)>0) || (list_store.erase(key)>0) || (hash_store.erase(key)>0);
    return false;
}
//expire
bool RedisDatabase::expire(std::string& key, std::string& seconds){
    std::lock_guard<std::mutex> lock(db_mutex);
    bool exists= (kv_store.find(key)!=kv_store.end())||(list_store.find(key)!=list_store.end()) || (hash_store.find(key)!=hash_store.end());
    if(!exists) return false;
    expiry_map[key]= std::chrono::steady_clock::now() + std::chrono::seconds(std::stoi(seconds));
    return true;
}
bool RedisDatabase::rename(std::string& oldKey, std::string& newKey){
    std::lock_guard<std::mutex> lock(db_mutex);
    bool found=false;
    if(kv_store.find(oldKey)!=kv_store.end()){
        kv_store[newKey]= kv_store[oldKey];
        kv_store.erase(oldKey);
        found= true;
    }

    if(list_store.find(oldKey)!=list_store.end()){
        list_store[newKey]= list_store[oldKey];
        list_store.erase(oldKey);
        found=true;
    }

    if(hash_store.find(oldKey)!=hash_store.end()){
        hash_store[newKey]= hash_store[oldKey];
        hash_store.erase(oldKey);
        found=true;
    }

    if(expiry_map.find(oldKey)!=expiry_map.end()){
        expiry_map[newKey]= expiry_map[oldKey];
        expiry_map.erase(oldKey);
    }

    return found;   

}


ssize_t RedisDatabase::llen(const std::string& key){
    std::lock_guard<std::mutex> lock(db_mutex);
    auto it = list_store.find(key);
    if(it!=list_store.end()){
        return it->second.size();
    }
    return 0;
}

void RedisDatabase::lpush(const std::string& key, const std::string& value){
    std::lock_guard<std::mutex> lock(db_mutex); 
    list_store[key].insert(list_store[key].begin(), value);     
}

void RedisDatabase::rpush(const std::string& key, const std::string& value){
    std::lock_guard<std::mutex> lock(db_mutex);
    list_store[key].push_back(value);
}

bool RedisDatabase::lpop(const std::string& key, std::string& value){
    std::lock_guard<std::mutex> lock(db_mutex);
    auto it= list_store.find(key);
    if(it!=list_store.end() && !it->second.empty()){
        value= it->second.front();
        it->second.erase(it->second.begin());
        return true;
    }
    return false;
}

bool RedisDatabase::rpop(const std::string& key, std::string& value){
    std::lock_guard<std::mutex> lock(db_mutex);
    auto it= list_store.find(key);
    if(it!=list_store.end() && !it->second.empty()){
        value= it->second.back();
        it->second.pop_back();
        return true;
    }
    return false;
}

int RedisDatabase::lrem(const std::string& key, int count,const std::string& value){
    std::lock_guard<std::mutex> lock(db_mutex);
    int removed= 0;
    auto it= list_store.find(key);
    if(it==list_store.end()){
        return 0;
    }
    auto& lst= it->second;
    if(count==0){
        auto new_end= std::remove(lst.begin(), lst.end(), value);
        removed= std::distance(new_end, lst.end());
    }else if(count>0){
        for(auto iter= lst.begin();iter!=lst.end() && removed<count;){
            if(*iter==value){
                iter= lst.erase(iter);
                removed++;
            }else{
                iter++;
            }
        }
    }else{
        for(auto riter=lst.rbegin();riter!=lst.rend() && removed < (-count);){
            if(*riter==value){
                auto fwdIter= riter.base();
                --fwdIter;
                fwdIter= lst.erase(fwdIter);
                ++removed;
                riter= std::reverse_iterator<std::vector<std::string>::iterator>(fwdIter);
            }else{
                ++riter;
            }
        }
    }
    return removed;

}

bool RedisDatabase::lindex(const std::string& key, int index, const std::string& value){
    std::lock_guard<std::mutex> lock(db_mutex);
    auto it= list_store.find(key);
    if(it==list_store.end()){
        return false;
    }

    auto& lst= it->second;
    if(index <0){
        index= lst.size()+index;
    }else if(index<0 || index >= static_cast<int>(lst.size())){
        return false;
    }
    lst[index]= value;
    return true;
}




bool RedisDatabase::lset(const std::string& key, int index, const std::string& value){
    std::lock_guard<std::mutex> lock(db_mutex);
    auto it= list_store.find(key);
    if(it==list_store.end()){
        return false;
    }

    auto& lst= it->second;
    if(index <0){
        index= lst.size()+index;
    }else if(index<0 || index >= static_cast<int>(lst.size())){
        return false;
    }
    lst[index]= value;
    return true;
}


bool hset(const std::string& key, const std::string& field, const std::string& value){
    std::lock_guard<std::mutex> lock(db_mutex);
    hash_store[key][field] = value;
    return true;
}
bool RedisDatabase::hget(const std::string& key, const std::string& field, std::string& value) {
    std::lock_guard<std::mutex> lock(db_mutex);
    auto it = hash_store.find(key);
    if (it != hash_store.end()) {
        auto f = it->second.find(field);
        if (f != it->second.end()) {
            value = f->second;
            return true;
        }
    }
    return false;
}

bool RedisDatabase::hexists(const std::string& key, const std::string& field) {
    std::lock_guard<std::mutex> lock(db_mutex);
    auto it = hash_store.find(key);
    if (it != hash_store.end())
        return it->second.find(field) != it->second.end();
    return false;
}

bool RedisDatabase::hdel(const std::string& key, const std::string& field) {
    std::lock_guard<std::mutex> lock(db_mutex);
    auto it = hash_store.find(key);
    if (it != hash_store.end())
        return it->second.erase(field) > 0;
    return false;
}

std::unordered_map<std::string, std::string> RedisDatabase::hgetall(const std::string& key) {
    std::lock_guard<std::mutex> lock(db_mutex);
    if (hash_store.find(key) != hash_store.end())
        return hash_store[key];
    return {};
}

std::vector<std::string> RedisDatabase::hkeys(const std::string& key) {
    std::lock_guard<std::mutex> lock(db_mutex);
    std::vector<std::string> fields;
    auto it = hash_store.find(key);
    if (it != hash_store.end()) {
        for (const auto& pair: it->second)
            fields.push_back(pair.first);
    }
    return fields;
}


std::vector<std::string> RedisDatabase::hvals(const std::string& key) {
    std::lock_guard<std::mutex> lock(db_mutex);
    std::vector<std::string> values;
    auto it = hash_store.find(key);
    if (it != hash_store.end()) {
        for (const auto& pair: it->second)
            values.push_back(pair.second);
    }
    return values;
}


ssize_t RedisDatabase::hlen(const std::string& key) {
    std::lock_guard<std::mutex> lock(db_mutex);
    auto it = hash_store.find(key);
    return (it != hash_store.end()) ? it->second.size() : 0;
}



bool RedisDatabase::hmset(const std::string& key, const std::vector<std::pair<std::string, std::string>>& fieldValues) {
    std::lock_guard<std::mutex> lock(db_mutex);
    for (const auto& pair: fieldValues) {
        hash_store[key][pair.first] = pair.second;
    }
    return true;
}

bool RedisDatabase::dump(const std::string& filename){
    std::lock_guard<std::mutex> lock(db_mutex);
    std::ofstream ofs(filename, std::ios::binary);
    if(!ofs) return false;
    for(auto &kv: kv_store){
        ofs << "K" << kv.first << " " << kv.second << "\n";
    }
    for(auto &list_kv: list_store){
        ofs << "L" << list_kv.first;
        for(auto &item: list_kv.second){
            ofs << " " << item;
        }
        ofs << "\n";
    }
    for(auto &hash_kv: hash_store){
        ofs << "H" << hash_kv.first;
        for(auto &field_val: hash_kv.second){
            ofs << " " << field_val.first << " " << field_val.second;
        }
        ofs << "\n";
    }
    return true;
}

bool RedisDatabase::load(const std::string& filename){
    std::lock_guard<std::mutex> lock(db_mutex);
    std::ifstream ifs(filename, std::ios::binary);
    if(!ifs) return false;
    kv_store.clear();
    list_store.clear();
    hash_store.clear();
    std::string line;
    while(std::getline(ifs, line)){
        std::istringstream iss(line);
        char type;
        iss >> type;
        if(type=='K'){
            std::string key, value;
            iss >> key >> value;
            kv_store[key]= value;
        }
        else if(type=='L'){
            std::string key;
            iss >> key;
            std::string item;
            std::vector<std::string> list;
            while(iss>>item){
                list.push_back(item);
            }
            list_store[key]=list;
        }
        else if(type=='H'){
            std::string key;
            iss>>key;
            std::unordered_map<std::string, std::string> hash;
            std::string pair;
            while(iss>>pair){
                auto pos= pair.find(':');
                if(pos!=std::string::npos){
                    std::string field= pair.substr(0, pos);
                    std::string value= pair.substr(pos+1);
                    hash[field]=value;
                }
            }
            hash_store[key]= hash;
        }
    }
    return true;
}
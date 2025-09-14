#ifndef REDIS_DATABASE_H
#define REDIS_DATABASE_H
#include <mutex>
#include <unordered_map>
#include <vector>
#include <string>
#include <chrono>
class RedisDatabase{
public:
    static RedisDatabase& getInstance();
    bool dump(const std::string& filename);
    bool load(const std::string& filename);
    bool flushAll();

    void set(std::string& key, std::string& value);
    bool get(std::string& key, std::string& value);
    std::vector<std::string> keys();
    std::string type(std::string& key);
    bool del(std::string& key);

    //expire
    bool expire(std::string& key, std::string& seconds);
    bool rename(std::string& oldKey, std::string& newKey);
    ssize_t llen(const std::string& key);
    void lpush(const std::string& key, const std::string& value);
    void rpush(const std::string& key, const std::string& value);
    bool lpop(const std::string& key, std::string& value);
    bool rpop(const std::string& key, std::string& value);
    int lrem(const std::string& key, int count,const std::string& value);
    bool lindex(const std::string& key, int index, const std::string& value);
    bool lset(const std::string& key, int index, const std::string& value);
   

    //hash operations
   bool hset(const std::string& key, const std::string& field, const std::string& value);
    bool hget(const std::string& key, const std::string& field, std::string& value);
    bool hexists(const std::string& key, const std::string& field);
    bool hdel(const std::string& key, const std::string& field);
    std::unordered_map<std::string, std::string> hgetall(const std::string& key);
    std::vector<std::string> hkeys(const std::string& key);
    std::vector<std::string> hvals(const std::string& key);
    ssize_t hlen(const std::string& key);
    bool hmset(const std::string& key, const std::vector<std::pair<std::string, std::string>>& fieldValues);


private:
    RedisDatabase() = default;
    ~RedisDatabase() = default;
    RedisDatabase(const RedisDatabase&) = delete;
    RedisDatabase& operator=(const RedisDatabase&) = delete;

    std::mutex db_mutex;
    std::unordered_map<std::string, std::string> kv_store;
    std::unordered_map<std::string, std::vector<std::string>> list_store;
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> hash_store;
    std::unordered_map<std::string, std::chrono::steady_clock::time_point> expiry_map;
};

#endif

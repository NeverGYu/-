#pragma once
#include "db_pool.h"

namespace sylar{
namespace db{

class MysqlUtil
{
public:
    static void init(const std::string& host, const std::string& user,
                        const std::string& password, const std::string& database,
                        size_t poolSize = 10)
    {
        db_pool::GetInstance()->init(host, user, password, database);
    }
    
    template<typename... Args>
    sql::ResultSet* executeQuery(const std::string& sql, Args&&... args)
    {
        auto conn = db_pool::GetInstance()->getConnection();
        return conn->executeQuery(sql, std::forward<Args>(args)...);
    }
    
    template<typename... Args>
    int executeUpdate(const std::string& sql, Args&&... args)
    {
        auto conn = db_pool::GetInstance()->getConnection();
        return conn->executeUpdate(sql, std::forward<Args>(args)...);
    }
};
    

}
}
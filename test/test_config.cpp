#include "config.h"
#include <iostream>
#include <vector>
#include <list>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>

/*-------------获得一个日志器----------------*/

sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

/*-------------------  获得各种类型的配置参数 ---------------------*/

sylar::ConfigVar<int>::ptr g_int = 
    sylar::Config::Lookup("system.port", "system port",(int)8080);

sylar::ConfigVar<float>::ptr g_intx = 
    sylar::Config::Lookup("system.port", "system port",(float)8080);

sylar::ConfigVar<float>::ptr g_float  = 
    sylar::Config::Lookup("system.value", "system value", (float)10.2f);

sylar::ConfigVar<std::vector<int>>::ptr g_int_vector_value_config = 
    sylar::Config::Lookup("system.int_vec","system int vec", std::vector<int>{1,2});


sylar::ConfigVar<std::list<int>>::ptr g_int_list_value_config = 
    sylar::Config::Lookup("system.int_list","system int list", std::list<int>{1,2});

sylar::ConfigVar<std::set<int>>::ptr g_int_set_value_config = 
    sylar::Config::Lookup("system.int_set","system int set", std::set<int>{1,2});

sylar::ConfigVar<std::unordered_set<int>>::ptr g_int_unordered_set_value_config = 
    sylar::Config::Lookup("system.int_unordered_set","system int unordered_set", std::unordered_set<int>{1,2});

sylar::ConfigVar<std::map<std::string, int>>::ptr g_string_int_map_value_config = 
    sylar::Config::Lookup("system.string_int_map","system string int map", std::map<std::string, int>{{"gch",1}});

sylar::ConfigVar<std::unordered_map<std::string, int>>::ptr g_string_int_umap_value_config = 
    sylar::Config::Lookup("system.string_int_umap","system string int umap", std::unordered_map<std::string, int>{{"th",2}});

/*-------------------  测试是否可以打印一个YAML::Node ---------------------*/
void print_yaml(const YAML::Node& node, int level)
{
    if (node.IsScalar())
    {
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << std::string(level * 4 , ' ')
            << node.Scalar() << " - " << node.Type() << " - " << level;
    }
    else if (node.IsNull())
    {
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << std::string(level * 4, ' ')
            <<" NULL - " << node.Type() << " - " << level;
    }
    else if (node.IsMap())
    {
        for (auto it = node.begin(); it != node.end(); ++it)
        {
            SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << std::string(level * 4, ' ')
                << it->first << " - " << it ->second.Type() << " - " << level;
            print_yaml(it->second, level + 1);  
        }
    }
    else if (node.IsSequence())
    {
        for (size_t i = 0; i < node.size(); ++i)
        {
            SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << std::string(level * 4 , ' ')
                << i << " - "  << node[i].Type() << " - " << level; 
            print_yaml(node[i] ,level +1 );
        }
        
    }
 
}


/*-------------------  测试是否可以打印一个YAML文件---------------------*/
void test_yaml()  
{
    YAML::Node root = YAML::LoadFile("/home/gch/高性能服务器框架/conf/testYAML");
    print_yaml(root, 0);
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << root.Scalar();
}


/*-------------------  测试不同类型的配置参数以及是否可以通过配置文件的修改来改变配置参数 ---------------------*/
void test_config()
{
    /*----------经过配置文件修改之前对应的参数值---------------------------------------------------------*/

    /*------- int 类型参数 ------*/
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "before: " << g_int->getValue();

    /*------- float 类型参数 ------*/
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "before: " << g_float->getValue();

    /*------- vector 类型参数 -------------------*/
    auto v = g_int_vector_value_config->getValue();
    for(auto& i :v)
    {
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "before int_vec: " << i ;
    }
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "before int_vec YAML: " << g_int_vector_value_config->toString(); 

    /*------- list 类型参数 -------------------*/
    auto l = g_int_list_value_config->getValue();
    for(auto& i :l)
    {
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "before int_list: " << i ;
    }
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "before int_list YAML: " << g_int_list_value_config->toString(); 

    /*-------  set 类型参数 -------------------*/
    auto s = g_int_set_value_config->getValue();
    for(auto& i :s)
    {
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "before int_set: " << i ;
    }
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "before int_set YAML: " << g_int_set_value_config->toString();

    /*-------  unordered_set 类型参数 -------------------*/
    auto us = g_int_unordered_set_value_config->getValue();
    for(auto& i :us)
    {
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "before int_unordered_set: " << i ;
    }
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "before int_unordered_set YAML: " << g_int_unordered_set_value_config->toString();

    /*-------  map 类型参数 -------------------*/
    auto m = g_string_int_map_value_config->getValue();
    for(auto it= m.begin(); it != m.end(); ++it)
    {
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "before string_int_map: " << "key: "<< it->first << ", value: " << it->second;
    }
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "before string_int_map YAML: " << g_string_int_map_value_config->toString();

    /*-------  unordered_map 类型参数 -------------------*/
    auto um = g_string_int_umap_value_config->getValue();
    for(auto it= um.begin(); it != um.end(); ++it)
    {
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "before string_int_umap: " << "key: "<< it->first << ", value: " << it->second;
    }
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "before string_int_umap YAML: " << g_string_int_umap_value_config->toString();
    
    /*--------- 获得配置文件并修改对应参数 -------------------------------*/
    YAML::Node root = YAML::LoadFile("/home/gch/高性能服务器框架/conf/testConfig");
    sylar::Config::LoadFromYaml(root);


    /*----------经过配置文件修改之后对应的参数值---------------------------------------------------------*/

    std::cout << std::endl;
    /*------- int 类型参数 ------*/
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "after: " << g_int->getValue();

    /*------- float 类型参数 ------*/
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "after: " << g_float->getValue();

    /*------- vector 类型参数 -------------------*/
    v = g_int_vector_value_config->getValue();
    for(auto& i :v)
    {
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "after int_vec: " << i ;
    }
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "after int_vec YAML: " << g_int_vector_value_config->toString(); 

    /*------- list 类型参数 -------------------*/
    l = g_int_list_value_config->getValue();
    for(auto& i :l)
    {
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "after int_list: " << i ;
    }
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "after int_list YAML: " << g_int_list_value_config->toString(); 

    /*-------  set 类型参数 -------------------*/
    s = g_int_set_value_config->getValue();
    for(auto& i :s)
    {
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "after int_set: " << i ;
    }
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "after int_set YAML: " << g_int_set_value_config->toString();

    /*-------  unordered_set 类型参数 -------------------*/
    us = g_int_unordered_set_value_config->getValue();
    for(auto& i :us)
    {
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "after int_unordered_set: " << i ;
    }
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "after int_unordered_set YAML: " << g_int_unordered_set_value_config->toString();

    /*-------  map 类型参数 -------------------*/
    m = g_string_int_map_value_config->getValue();
    for(auto it= m.begin(); it != m.end(); ++it)
    {
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "after string_int_map: " << "key: "<< it->first << ", value: " << it->second;
    }
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "after string_int_map YAML: " << g_string_int_map_value_config->toString();

    /*-------  unordered_map 类型参数 -------------------*/
    um = g_string_int_umap_value_config->getValue();
    for(auto it= um.begin(); it != um.end(); ++it)
    {
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "after string_int_umap: " << "key: "<< it->first << ", value: " << it->second;
    }
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "after string_int_umap YAML: " << g_string_int_umap_value_config->toString();
}


/*------------------------------ 测试自定义类 ----------------------------------*/
class Person
{
public:
    Person() {}

    std::string m_name;
    int m_age = 0;
    bool m_sex = 0;

    bool operator==(const Person &oth) const {
        return m_name == oth.m_name && m_age == oth.m_age && m_sex == oth.m_sex;
    }

    std::string toString() const
    {
        std::stringstream ss;
        ss << "[Person name= " << m_name
           << " age=" << m_age
           << " sex=" << m_sex
           << "]" ;
        return ss.str();
    }
    

};

namespace sylar
{
template<>
class lexicalCast<std::string, Person>
{
public:
    Person operator()(const std::string& val)
    {
        YAML::Node node = YAML::Load(val);
        Person p;
        p.m_name = node["name"].as<std::string>();
        p.m_age = node["age"].as<int>();
        p.m_sex = node["sex"].as<bool>();
        return p;
    }
};

template<>
class lexicalCast<Person, std::string>
{
public:
    std::string operator()(const Person& p) const
    {
        YAML::Node node;
        node["name"] = p.m_name;
        node["age"] = p.m_age;
        node["sex"] = p.m_sex;
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

}

sylar::ConfigVar<Person>::ptr g_person 
    = sylar::Config::Lookup("class.person", "system Person", Person());

sylar::ConfigVar<std::map<std::string, Person>>::ptr g_string_person_map 
    = sylar::Config::Lookup("class.map", "system Person", std::map<std::string, Person>());  

sylar::ConfigVar<std::map<std::string, std::vector<Person>>>::ptr g_string_vector_map
    = sylar::Config::Lookup("class.vec_map", "class vec_map", std::map<std::string, std::vector<Person>>());


void test_class()
{   
    /*----------经过配置文件修改之前对应的参数值---------------------------------------------------------*/

    /*-----------------------测试不加容器的自定义类---------------------------*/
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "before: "<<g_person->getValue().toString() << " - " << g_person->toString();
    
    /*-----------------------测试加容器的自定义类---------------------------*/
    auto m = g_string_person_map->getValue();
    for (auto& i : m)
    {
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "class.map before: " << i.first << " - " << i.second.toString();
    }
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) <<  "class.map before: size=" << m.size() ;  

   SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "before class.vec_map: " << g_string_vector_map->toString();

    /*-------------------------测试变更事件(注册回调事件)----------------------------------------------*/
    g_person->addlistener([](const Person& oldValue, const Person& newValue){
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "oldValue=" << oldValue.toString()
                << " newValue=" << newValue.toString();
    });

    /*----------经过配置文件修改之后对应的参数值---------------------------------------------------------*/

    /*--------- 获得配置文件并修改对应参数 -------------------------------*/
    YAML::Node root = YAML::LoadFile("/home/gch/高性能服务器框架/conf/testConfig");
    sylar::Config::LoadFromYaml(root);

    /*-----------------------测试不加容器的自定义类---------------------------*/
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "after: "<<g_person->getValue().toString() << " - " << g_person->toString();

    /*-----------------------测试加容器的自定义类---------------------------*/
    m = g_string_person_map->getValue();
    for (auto& i : m)
    {
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "class.map after: " << i.first << " - " << i.second.toString();
    }
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) <<  "class.map after: size=" << m.size() ;  

    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "after class.vec_map: " << g_string_vector_map->toString();

}

void test_log()
{
    static sylar::Logger::ptr system_log = SYLAR_LOG_NAME("system");
    SYLAR_LOG_INFO(system_log) << "hello system" << std::endl;
    auto appenders = system_log->getAppenders();
    std::cout << "Appender count: " << appenders.size() << std::endl;
    for (auto &appender : appenders)
    {
        std::cout << "Appender: " << appender->toYamlString() << std::endl;
    }
    std::cout << sylar::LoggerMgr::GetInstance()->toYamlString() << std::endl;
    YAML::Node root = YAML::LoadFile("/home/gch/高性能服务器框架/conf/log.yaml");
    sylar::Config::LoadFromYaml(root);
    std::cout<<"==================================="<<std::endl;
    std::cout << sylar::LoggerMgr::GetInstance()->toYamlString() << std::endl;
    std::cout<<"==================================="<<std::endl;
    std::cout << root << std::endl;
}


/*-------------------  执行打印 ---------------------*/
int main()
{
    // test_yaml();

    // test_config();

    // test_class();

    test_log();

    return 0;
}
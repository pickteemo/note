---
title: Protobuf Reflection Note
date: 2021-03-17 14:19:24
tags:
---

#  ProtoBuf Reflection Note

参考：[官方文档 Descriptor](https://developers.google.com/protocol-buffers/docs/reference/cpp/google.protobuf.descriptor#DescriptorPool.generated_pool.details)

 [一种自动反射消息类型的 Google Protobuf 网络传输方案](https://www.cnblogs.com/Solstice/archive/2011/04/03/2004458.html)

Proto使用过程中，有这样几个问题：

1. 对proto中已有的内容进行修改/删除非常不方便，例如将proto中的一个字段foo删除，需要：
   1. 在.proto里把foo删掉并重新编译，
   2. 在工程中删除所有的foo(),set_foo(),mutable_foo()等,否则编译会挂掉。

   尤其当foo不仅仅是一个变量时，或者该proto同时被很多工程使用时，会非常麻烦。
   
2. 拿到了一个pb对象，希望遍历该对象的所有字段(反序列化）。

   例如定义一个pb message如下：

   ```protobuf
   Person person;
   person.set_name("yingshin");
   person.set_age(21);
   ```

   希望将该对象自动转为json格式的字符串：

   ```json
   {
       "name":"yingshin",
       "age":21
   }
   ```

   如果添加了新的字段，输出也能自动更新。



### 反射

protobuf提供了一种反射机制，能够动态地调用对象并获取信息。利用protobuf的反射机制，可以帮助优化以上两种场景。

<!-- more -->

相关的类：

![Message](https://izualzhy.cn/assets/images/pb-reflection.png)

| Class           | Description                                                  |
| --------------- | ------------------------------------------------------------ |
| Message         | pb对象                                                       |
| Descriptor      | 对 Message 进行描述，包括 message 的名字、所有字段的描述、原始 proto 文件内容等 |
| FieldDescriptor | 对 Message 中单个字段进行描述，包括字段名、字段属性、原始的 field 字段等 |
| Reflection      | 提供了动态读和写 message 中单个字段能力                      |

在实际应用时，我们拿到一个pb对象后，可以：

通过Descriptor获得其每个字段的name和type

通过Reflection中的GetX获得具体的字段



常规的流程是，收到数据包，构造一个pb对象，再反序列化

利用反射，我们可以使用Descriptor和Refelction来进行动态解析:

- 使用FindMessageTypebyName() 获得descriptor，来代替手动构造pb对象

- 使用FindFiledByName("id")来代替具体的set_id()函数



### 实例 - 获取Descriptor 和Reflection

对于proto文件：

```protobuf
package T;
message Test
{
    optional int32 id = 1;
}
```

首先我们需要获取Descriptor，最常见的是调用message的GetDescriptor方法

```c++
//直接从message获取，
T::Test test;
auto descriptor = test.GetDescriptor() ;
auto reflecter = test.GetReflection() ;
// 拿到属性的描述包.
auto field = descriptor->FindFieldByName("id");
// 设置属性的值.
reflecter->SetInt32(&test , field , 5 ) ;
// 获取属性的值.
std::cout<<reflecter->GetInt32(test , field)<< std::endl ;
```

除了调用message的GetDescriptor方法之外，一般有两种方式来获取Descriptor：动态编译与静态编译获取

#### 动态编译

使用protobuf的动态编译机制，在运行时对某个proto文件进行动态编译，从而得到其所有元数据(descriptor):

```c++
DiskSourceTree sourceTree;
//目录映射> look up .proto file in current directory
sourceTree.MapPath("", "./");
Importer importer(&sourceTree, NULL);
//runtime compile foo.proto
importer.Import("foo.proto");
 
//拿到Descriptor
const Descriptor *descriptor1 = importer.pool()->FindMessageTypeByName("Test.Foo");
// 创建一个动态的消息工厂.
google::protobuf::DynamicMessageFactory factory;
// 从消息工厂中创建出一个类型原型.
auto proto1 = factory.GetPrototype(descriptor1);
// 构造一个可用的消息.
auto message1= proto1->New();
// 通过反射接口给字段赋值.
auto reflection1 = message1->GetReflection();
auto filed1 = descriptor1->FindFieldByName("id");
reflection1->SetInt32(message1,filed1,1);

std::cout << message1->DebugString();
delete message1 ;
```

> Note:我在尝试动态编译foo.proto时，如果foo.proto有import其他目录下的proto，会提示找不到文件，还没找到解决问题的方法。

#### 静态编译

在proto生成的pb.h/cc的构造函数中，会调用静态类static void MessageFactory::InternalRegisterGeneratedFile将自己注册到DescriptorPool::generated_pool中，后续可以直接使用generated_pool拿到Descriptor。

```c++
T::Test xxx; //需要调用一次注册到generated_pool中
std::string str;
xxx.SerializeToString(str);

auto descriptor = google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName("T.Test");
if (nullptr == descriptor) return 0；

// 利用Descriptor拿到类型注册的instance
auto prototype = google::protobuf::MessageFactory::generated_factory()->GetPrototype(descriptor);
if ( nullptr == descriptor) return 0;

// 构造一个可用的消息.
auto message = prototype->New();
// 从序列化好的str中进行反序列化
message->ParseFromString(str);
// 从message中拿到Descriptor和Reflection
const google::protobuf::Reflection* reflection = message.GetReflection();
const google::protobuf::Descriptor* descriptor = message.GetDescriptor();

delete message ;
return 0 ;
```



### 一些工程中的应用实例

参考：

https://izualzhy.cn/protobuf-message-reflection

http://arganzheng.life/reflection-of-protobuf.html

https://cloud.tencent.com/developer/article/1753977

#### 自动序列化及反序列化

#### serialize_message

serialize_message遍历提取message中各个字段以及对应的值，序列化到string中。
主要思路就是通过Descriptor得到每个字段的描述符：字段名、字段的cpp类型。
通过Reflection的GetX接口获取对应的value。

```c++
void serialize_message(const google::protobuf::Message& message, std::string* serialized_string) {
    const google::protobuf::Descriptor* descriptor = message.GetDescriptor();
    const google::protobuf::Reflection* reflection = message.GetReflection();

    for (int i = 0; i < descriptor->field_count(); ++i) {
        const google::protobuf::FieldDescriptor* field = descriptor->field(i);
        bool has_field = reflection->HasField(message, field);

        if (has_field) {
            //arrays not supported
            assert(!field->is_repeated());

            switch (field->cpp_type()) {
#define CASE_FIELD_TYPE(cpptype, method, valuetype)\
                case google::protobuf::FieldDescriptor::CPPTYPE_##cpptype:{\
                    valuetype value = reflection->Get##method(message, field);\
                    int wsize = field->name().size();\
                    serialized_string->append(reinterpret_cast<char*>(&wsize), sizeof(wsize));\
                    serialized_string->append(field->name().c_str(), field->name().size());\
                    wsize = sizeof(value);\
                    serialized_string->append(reinterpret_cast<char*>(&wsize), sizeof(wsize));\
                    serialized_string->append(reinterpret_cast<char*>(&value), sizeof(value));\
                    break;\
                }

                CASE_FIELD_TYPE(INT32, Int32, int);
                CASE_FIELD_TYPE(UINT32, UInt32, uint32_t);
                CASE_FIELD_TYPE(FLOAT, Float, float);
                CASE_FIELD_TYPE(DOUBLE, Double, double);
                CASE_FIELD_TYPE(BOOL, Bool, bool);
                CASE_FIELD_TYPE(INT64, Int64, int64_t);
                CASE_FIELD_TYPE(UINT64, UInt64, uint64_t);
#undef CASE_FIELD_TYPE
                case google::protobuf::FieldDescriptor::CPPTYPE_ENUM: {
                    int value = reflection->GetEnum(message, field)->number();
                    int wsize = field->name().size();
                    //写入name占用字节数
                    serialized_string->append(reinterpret_cast<char*>(&wsize), sizeof(wsize));
                    //写入name
                    serialized_string->append(field->name().c_str(), field->name().size());
                    wsize = sizeof(value);
                    //写入value占用字节数
                    serialized_string->append(reinterpret_cast<char*>(&wsize), sizeof(wsize));
                    //写入value
                    serialized_string->append(reinterpret_cast<char*>(&value), sizeof(value));
                    break;
                }
                case google::protobuf::FieldDescriptor::CPPTYPE_STRING: {
                    std::string value = reflection->GetString(message, field);
                    int wsize = field->name().size();
                    serialized_string->append(reinterpret_cast<char*>(&wsize), sizeof(wsize));
                    serialized_string->append(field->name().c_str(), field->name().size());
                    wsize = value.size();
                    serialized_string->append(reinterpret_cast<char*>(&wsize), sizeof(wsize));
                    serialized_string->append(value.c_str(), value.size());
                    break;
                }
                case google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE: {
                    std::string value;
                    int wsize = field->name().size();
                    serialized_string->append(reinterpret_cast<char*>(&wsize), sizeof(wsize));
                    serialized_string->append(field->name().c_str(), field->name().size());
                    const google::protobuf::Message& submessage = reflection->GetMessage(message, field);
                    serialize_message(submessage, &value);
                    wsize = value.size();
                    serialized_string->append(reinterpret_cast<char*>(&wsize), sizeof(wsize));
                    serialized_string->append(value.c_str(), value.size());
                    break;
                }
            }
        }
    }
}
```

#### parse_message

parse_message通过读取field/value，还原message对象。
主要思路跟serialize_message很像，通过Descriptor得到每个字段的描述符FieldDescriptor，通过Reflection的SetX填充message。

```c++
void parse_message(const std::string& serialized_string, google::protobuf::Message* message) {
    const google::protobuf::Descriptor* descriptor = message->GetDescriptor();
    const google::protobuf::Reflection* reflection = message->GetReflection();
    std::map<std::string, const google::protobuf::FieldDescriptor*> field_map;

    for (int i = 0; i < descriptor->field_count(); ++i) {
        const google::protobuf::FieldDescriptor* field = descriptor->field(i);
        field_map[field->name()] = field;
    }

    const google::protobuf::FieldDescriptor* field = NULL;
    size_t pos = 0;
    while (pos < serialized_string.size()) {
        int name_size = *(reinterpret_cast<const int*>(serialized_string.substr(pos, sizeof(int)).c_str()));
        pos += sizeof(int);
        std::string name = serialized_string.substr(pos, name_size);
        pos += name_size;

        int value_size = *(reinterpret_cast<const int*>(serialized_string.substr(pos, sizeof(int)).c_str()));
        pos += sizeof(int);
        std::string value = serialized_string.substr(pos, value_size);
        pos += value_size;

        std::map<std::string, const google::protobuf::FieldDescriptor*>::iterator iter =
            field_map.find(name);
        if (iter == field_map.end()) {
            fprintf(stderr, "no field found.\n");
            continue;
        } else {
            field = iter->second;
        }

        assert(!field->is_repeated());
        switch (field->cpp_type()) {
#define CASE_FIELD_TYPE(cpptype, method, valuetype)\
            case google::protobuf::FieldDescriptor::CPPTYPE_##cpptype: {\
                reflection->Set##method(\
                        message,\
                        field,\
                        *(reinterpret_cast<const valuetype*>(value.c_str())));\
                std::cout << field->name() << "\t" << *(reinterpret_cast<const valuetype*>(value.c_str())) << std::endl;\
                break;\
            }
            CASE_FIELD_TYPE(INT32, Int32, int);
            CASE_FIELD_TYPE(UINT32, UInt32, uint32_t);
            CASE_FIELD_TYPE(FLOAT, Float, float);
            CASE_FIELD_TYPE(DOUBLE, Double, double);
            CASE_FIELD_TYPE(BOOL, Bool, bool);
            CASE_FIELD_TYPE(INT64, Int64, int64_t);
            CASE_FIELD_TYPE(UINT64, UInt64, uint64_t);
#undef CASE_FIELD_TYPE
            case google::protobuf::FieldDescriptor::CPPTYPE_ENUM: {
                const google::protobuf::EnumValueDescriptor* enum_value_descriptor =
                    field->enum_type()->FindValueByNumber(*(reinterpret_cast<const int*>(value.c_str())));
                reflection->SetEnum(message, field, enum_value_descriptor);
                std::cout << field->name() << "\t" << *(reinterpret_cast<const int*>(value.c_str())) << std::endl;
                break;
            }
            case google::protobuf::FieldDescriptor::CPPTYPE_STRING: {
                reflection->SetString(message, field, value);
                std::cout << field->name() << "\t" << value << std::endl;
                break;
            }
            case google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE: {
                google::protobuf::Message* submessage = reflection->MutableMessage(message, field);
                parse_message(value, submessage);
                break;
            }
            default: {
                break;
            }
        }
    }
}
```

#### 获取 PB 中所有非空字段

在业务中，经常会需要获取某个 Message 中所有非空字段，形成一个 map<string,string>，使用 PB 反射写法如下：

```c++
#include "pb_util.h"

#include <sstream>

namespace comm_tools {
int PbToMap(const google::protobuf::Message &message,
            std::map<std::string, std::string> &out) {
#define CASE_FIELD_TYPE(cpptype, method, valuetype)                            \
  case google::protobuf::FieldDescriptor::CPPTYPE_##cpptype: {                 \
    valuetype value = reflection->Get##method(message, field);                 \
    std::ostringstream oss;                                                    \
    oss << value;                                                              \
    out[field->name()] = oss.str();                                            \
    break;                                                                     \
  }

#define CASE_FIELD_TYPE_ENUM()                                                 \
  case google::protobuf::FieldDescriptor::CPPTYPE_ENUM: {                      \
    int value = reflection->GetEnum(message, field)->number();                 \
    std::ostringstream oss;                                                    \
    oss << value;                                                              \
    out[field->name()] = oss.str();                                            \
    break;                                                                     \
  }

#define CASE_FIELD_TYPE_STRING()                                               \
  case google::protobuf::FieldDescriptor::CPPTYPE_STRING: {                    \
    std::string value = reflection->GetString(message, field);                 \
    out[field->name()] = value;                                                \
    break;                                                                     \
  }

  const google::protobuf::Descriptor *descriptor = message.GetDescriptor();
  const google::protobuf::Reflection *reflection = message.GetReflection();

  for (int i = 0; i < descriptor->field_count(); i++) {
    const google::protobuf::FieldDescriptor *field = descriptor->field(i);
    bool has_field = reflection->HasField(message, field);

    if (has_field) {
      if (field->is_repeated()) {
        return -1; // 不支持转换repeated字段
      }

      const std::string &field_name = field->name();
      switch (field->cpp_type()) {
        CASE_FIELD_TYPE(INT32, Int32, int);
        CASE_FIELD_TYPE(UINT32, UInt32, uint32_t);
        CASE_FIELD_TYPE(FLOAT, Float, float);
        CASE_FIELD_TYPE(DOUBLE, Double, double);
        CASE_FIELD_TYPE(BOOL, Bool, bool);
        CASE_FIELD_TYPE(INT64, Int64, int64_t);
        CASE_FIELD_TYPE(UINT64, UInt64, uint64_t);
        CASE_FIELD_TYPE_ENUM();
        CASE_FIELD_TYPE_STRING();
      default:
        return -1; // 其他异常类型
      }
    }
  }

  return 0;
}
} // namespace comm_tools
```

通过上面的代码，如果需要在 proto 中增加字段，不再需要修改原来的代码。

> repeated类型的数据比较特殊，可以单独搞一个List来存

#### [Self-describing Messages](https://developers.google.com/protocol-buffers/docs/techniques?hl=zh-CN#self-description)


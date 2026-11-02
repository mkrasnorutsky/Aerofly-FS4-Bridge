#pragma once
#include "tm_external_message.h"
#include "message_catalog.h"
#include <nlohmann/json.hpp>
#include <cmath>
#include <cstring>
#include <codecvt>
#include <locale>
#include <string>
#include <vector>
#include "command_aliases.h"

// One entry per SDK variant, not per name: Trim Value/Step/Move must coexist.
struct CommandDefinition {
    const char* symbol;
    const char* name;
    tm_msg_data_type type;
    tm_msg_flag flag;
    tm_msg_access access;
    tm_msg_unit unit;
};
inline tm_uint64 CommandMessageID(const char* name) {
    tm_uint64 hash=14695981039346656037ULL;
    do { hash^=static_cast<unsigned char>(*name); hash*=1099511628211ULL; } while(*name++);
    return hash;
}
inline const std::vector<CommandDefinition>& CommandCatalog() {
    static const std::vector<CommandDefinition> entries = {
#define F(symbol,name,type,flag,access,unit,comment) {#symbol,name,type,flag,access,unit},
        MESSAGE_LIST(F)
        AIRCRAFT_SPECIFIC_MESSAGE_LIST(F)
#undef F
    };
    return entries;
}
inline const char* CommandFlagName(tm_msg_flag flag) {
    switch(flag) {
    case tm_msg_flag::Value: return "Value";
    case tm_msg_flag::Step: return "Step";
    case tm_msg_flag::Move: return "Move";
    case tm_msg_flag::Toggle: return "Toggle";
    case tm_msg_flag::Event: return "Event";
    default: return "None";
    }
}
class CommandProcessor {
public:
    tm_external_message ParseCommand(const std::string& text) const {
        try {
            const auto json = nlohmann::json::parse(text);
            if(!json.is_object() || !json.contains("variable") || !json["variable"].is_string() || !json.contains("value")) return {};
            const auto name=CommandAlias(json["variable"].get<std::string>());
            std::string flag;
            if(json.contains("flag")) {
                if(!json["flag"].is_string()) return {};
                flag=json["flag"].get<std::string>();
                if(flag.empty()) return {};
            }
            const CommandDefinition* selected=nullptr;
            for(const auto& entry:CommandCatalog()) {
                if(entry.access!=tm_msg_access::Write && entry.access!=tm_msg_access::ReadWrite) continue;
                // SDK symbolic names also provide unambiguous aliases, e.g. ControlsTrimStep.
                if(name!=entry.name && name!=entry.symbol) continue;
                if(!flag.empty() && flag!=CommandFlagName(entry.flag)) continue;
                if(!selected || (flag.empty() && entry.flag==tm_msg_flag::Value)) selected=&entry;
            }
            if(!selected) return {};
            const auto& d=*selected;
            tm_external_message message(tm_string_hash(CommandMessageID(d.name)),d.type,d.flag,d.access,d.unit);
            const auto& value=json["value"];
            if(d.type==tm_msg_data_type::Double) {
                if(!value.is_number()) return {};
                const double number=value.get<double>();
                if(!std::isfinite(number)) return {};
                message.SetValue(number);
            } else if(d.type==tm_msg_data_type::Vector3d || d.type==tm_msg_data_type::Vector4d) {
                const size_t count=d.type==tm_msg_data_type::Vector3d?3:4;
                if(!value.is_array() || value.size()!=count) return {};
                double v[4]={};
                for(size_t i=0;i<count;++i) {
                    if(!value[i].is_number()) return {};
                    v[i]=value[i].get<double>();
                    if(!std::isfinite(v[i])) return {};
                }
                if(count==3) message.SetValue(tm_vector3d(v[0],v[1],v[2]));
                else message.SetValue(tm_vector4d(v[0],v[1],v[2],v[3]));
            } else if(d.type==tm_msg_data_type::String) {
                if(!value.is_string()) return {};
                const auto utf8=value.get<std::string>();
                if(utf8.find('\0')!=std::string::npos) return {};
                std::wstring_convert<std::codecvt_utf8_utf16<char16_t>,char16_t> converter;
                const auto utf16=converter.from_bytes(utf8);
                const size_t bytes=(utf16.size()+1)*sizeof(char16_t);
                if(bytes>message.GetMaxDataSize()) return {};
                auto header=message.GetHeader();
                header.MessageSize=static_cast<tm_uint16>(sizeof(tm_msg_header)+bytes);
                message=tm_external_message(header);
                std::memcpy(message.GetDataPointer(),utf16.c_str(),bytes);
            } else return {};
            // No universal 0..1 clamp: steps can be negative, frequencies exceed 1.
            return message;
        } catch(const std::exception&) { return {}; }
    }
    std::vector<tm_external_message> ProcessCommands(const std::vector<std::string>& commands) const {
        std::vector<tm_external_message> messages;
        for(const auto& text:commands) {
            auto message=ParseCommand(text);
            if(message.GetDataType()!=tm_msg_data_type::None) messages.push_back(message);
        }
        return messages;
    }
};

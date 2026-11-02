#include "command_processor.h"
#include <iostream>
#include <stdexcept>

static void check(bool ok,const std::string& what) { if(!ok) throw std::runtime_error(what); }
int main(int argc,char** argv) {
    try {
        CommandProcessor processor;
        if(argc==2 && std::string(argv[1])=="--manifest") {
            nlohmann::json manifest=nlohmann::json::array();
            for(const auto& d:CommandCatalog()) {
                if(d.access!=tm_msg_access::Write && d.access!=tm_msg_access::ReadWrite) continue;
                nlohmann::json value=-.375;
                if(d.type==tm_msg_data_type::Vector3d) value={1.0,-2.0,3.0};
                if(d.type==tm_msg_data_type::Vector4d) value={1.0,0.0,0.0,0.0};
                if(d.type==tm_msg_data_type::String) value="F2G123";
                manifest.push_back({{"variable",d.name},{"flag",CommandFlagName(d.flag)},
                    {"value",value},{"id",CommandMessageID(d.name)},
                    {"type",static_cast<unsigned>(d.type)},{"flags",static_cast<tm_uint64>(d.flag)}});
            }
            std::cout<<manifest.dump(); return 0;
        }
        check(CommandMessageID("Controls.Trim")==tm_string_hash("Controls.Trim").GetHash(),"Runtime hash differs from SDK");
        size_t tested=0;
        for(const auto& d:CommandCatalog()) {
            nlohmann::json value=-0.375;
            if(d.type==tm_msg_data_type::Vector3d) value={1.0,-2.0,3.0};
            if(d.type==tm_msg_data_type::Vector4d) value={1.0,0.0,0.0,0.0};
            if(d.type==tm_msg_data_type::String) value="F2G123";
            nlohmann::json request={{"variable",d.name},{"value",value},{"flag",CommandFlagName(d.flag)}};
            const auto result=processor.ParseCommand(request.dump());
            const bool writable=d.access==tm_msg_access::Write || d.access==tm_msg_access::ReadWrite;
            if(!writable) {
                // A read-only variant may share a name/flag with a writable variant.
                bool shared=false;
                for(const auto& other:CommandCatalog()) if(std::string(other.name)==d.name && other.flag==d.flag &&
                    (other.access==tm_msg_access::Write || other.access==tm_msg_access::ReadWrite)) shared=true;
                if(!shared) check(result.GetDataType()==tm_msg_data_type::None,std::string("Read-only accepted: ")+d.name);
                continue;
            }
            check(result.GetID()==CommandMessageID(d.name),std::string("Missing command: ")+d.symbol);
            check(result.GetDataType()==d.type,std::string("Wrong type: ")+d.symbol);
            check(result.GetFlags().IsSet(tm_msg_flag::Write),std::string("Missing write access: ")+d.symbol);
            if(d.flag!=tm_msg_flag::None) check(result.GetFlags().IsSet(d.flag),std::string("Wrong flag: ")+d.symbol);
            if(d.type==tm_msg_data_type::Double) check(result.GetDouble()==-.375,std::string("Value altered: ")+d.symbol);
            request["variable"]=d.symbol;
            check(processor.ParseCommand(request.dump()).GetID()==result.GetID(),std::string("Missing alias: ")+d.symbol);
            ++tested;
        }
        auto trim=processor.ParseCommand(R"({"variable":"Controls.Trim","value":-0.6})");
        check(trim.GetDouble()==-.6 && trim.GetFlags().IsSet(tm_msg_flag::Value),"Default trim must be absolute");
        auto step=processor.ParseCommand(R"({"variable":"Controls.Trim","flag":"Step","value":-1})");
        check(step.GetDouble()==-1 && step.GetFlags().IsSet(tm_msg_flag::Step),"Trim step lost");
        auto parking=processor.ParseCommand(R"({"variable":"Controls.ParkingBrake","value":1})");
        check(parking.GetFlags().IsSet(tm_msg_flag::Toggle),"Parking brake must keep SDK toggle semantics");
        auto vector=processor.ParseCommand(R"({"variable":"View.FreePosition","value":[1,-2,3]})");
        check(vector.GetVector3d().y==-2,"Vector payload damaged");
        auto flight=processor.ParseCommand(R"({"variable":"FlightManagementSystem.FlightNumber","value":"F2G123"})");
        check(flight.GetDataSize()==14 && reinterpret_cast<const tm_chartype*>(flight.GetDataPointer())[0]=='F',"UTF16 payload damaged");
        check(processor.ParseCommand(nlohmann::json({{"variable","FlightManagementSystem.FlightNumber"},{"value",std::string(32,'A')}}).dump()).GetDataType()==tm_msg_data_type::None,"Oversized SDK string accepted");
        check(processor.ParseCommand(R"({"variable":"Controls.PropellerSpeed.1","value":0.75})").GetDouble()==.75,"Legacy propeller alias lost");
        for(const auto* bad:{"{}","null", "[]", "{bad}",
            R"({"variable":"Controls.Trim","value":null})",
            R"({"variable":"Controls.Trim","value":true})",
            R"({"variable":"Controls.Trim","value":0.2oops})",
            R"({"variable":"Controls.Trim","flag":"Unknown","value":1})",
            R"({"variable":"Controls.Trim","value":1e999})",
            R"({"variable":"Controls.Trim","value":1} trailing)",
            R"({"variable":"Aircraft.PitchTrim","value":1})",
            R"({"variable":"View.FreePosition","value":[1,2]})",
            R"({"variable":"View.FreePosition","value":[1,"2",3]})",
            R"({"variable":"Unknown.Control","value":1})"})
            check(processor.ParseCommand(bad).GetDataType()==tm_msg_data_type::None,std::string("Invalid request accepted: ")+bad);
        std::cout<<"Passed "<<tested<<" writable SDK variants, aliases, types, flags, and malformed input checks.\n";
        return 0;
    } catch(const std::exception& e) { std::cerr<<e.what()<<'\n'; return 1; }
}

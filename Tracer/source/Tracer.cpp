/*
 *  Tracer.cpp
 *  LPTracer
 *
 *  http://www.logicpoet.com
 *
 *  Created by Jeff Wilcox on 6/29/08.
 *  Copyright 2008 Logic Poet. All rights reserved.
 * 
 *  The MIT License
 *  Permission is hereby granted, free of charge, to any person
 *  obtaining a copy of this software and associated documentation
 *  files (the "Software"), to deal in the Software without
 *  restriction, including without limitation the rights to use,
 *  copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the
 *  Software is furnished to do so, subject to the following
 *  conditions:
 *
 *  The above copyright notice and this permission notice shall be
 *  included in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 *  OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 *  HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 *  OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include "Tracer.h"
#include <iomanip>

using namespace lpt;

#pragma mark -
#pragma mark Constructors, Destructors & Singleton Management

//Handle singleton instance
//Note: this is only safe for single threaded apps, but since systemc is single threaded thats ok
//Needs to be this way to ensure that the destructor gets called for the singleton on app exit
Tracer* Tracer::getSharedTracer(){
    static Tracer inst;
    return &inst;
}

Tracer::Tracer(){
    traceCount = 0;
    moduleCount = 0;
    eventTypeCount = 0;
    initComplete = false;
    this->filename = "tracefile.scnx";
}

Tracer::Tracer(char *filename){
    traceCount = 0;
    moduleCount = 0;
    eventTypeCount = 0;
    initComplete = false;
    this->filename = filename;
}

Tracer::~Tracer(){
    if (outfile){
        outfile << "</document>\n";
        outfile.flush();
        outfile.close();
    }
}

//This initializes the file.  Gets called the first time an event is marked to ensure elaboration has completed.
void Tracer::initialize(){
    outfile.open(filename.c_str());
    if (!outfile){
        cout << "***Tracer Error*** Cannot Open Trace File : " << filename << endl;
        sc_stop();
    }
    outfile << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
    outfile << "<!DOCTYPE document PUBLIC \"-//LOGICPOET//DTD Scansion Tracefile version 0.7//EN\"\n";
    outfile << "\"http://www.logicpoet.com/DTD/scansion.dtd\" >\n";
    outfile << "<document>\n";
    registerAllModules();
    initComplete = true;
}

#pragma mark -
#pragma mark Filename Accessors
void Tracer::setFilename(string filename){
    if (!initialized){
        //First check to make sure that an extension was set and add one if it is missing
        int pos = filename.find(".scnx");
        bool addExtension = false;
        if (pos == string::npos) addExtension = true; //No extension found
        if (pos != filename.size()-5) addExtension = true;  //Found it,but not in the right spot, so add another one        
        this->filename = filename;
        if (addExtension) this->filename += ".scnx";
    }
    else {
        cout << "***Tracer Warning*** Attempted to assign filename " << filename << "after trace recording has started\n.";
        cout << "Ignoring new filename.  File recorded to: " << this->filename << endl;
    }
    
}
void Tracer::setSharedFilename(string filename){
    getSharedTracer()->setFilename(filename);
}
string Tracer::getFilename(){
    return filename;
}
string Tracer::getSharedFilename(){
    return getSharedTracer()->getFilename();
}

#pragma mark -
#pragma mark Module Methods
void Tracer::registerAllModules(){
    sc_simcontext* context = sc_core::sc_get_curr_simcontext();
    sc_object* obj = context->first_object();
    while (obj){
        //Find the top level modules to start things off
        if (!obj->get_parent()){
            if (obj->kind() == "sc_module"){
                registerModule(obj);
            }         
        }
        obj = context->next_object();
    }
}

void Tracer::registerModule(sc_object* mod){
    int index = ++moduleCount;
    //Start the module definition
    outfile << "<module id=\"M" << index << "\" name=\"" << mod->basename() << "\">\n";
    moduleIdMap.insert(std::pair<sc_module*, int>((sc_module *)mod, index));
    //Register the child modules if they exist
    vector<sc_object*> children = ((sc_module *)mod)->get_child_objects();
    for(int i = 0; i < children.size(); i++){
        if (children[i]->kind() == "sc_module"){
            registerModule(children[i]);
        } 
    }
    //End the module
    outfile << "</module>\n";
}


int Tracer::getModuleId(sc_module* module){
    map<sc_module *, int>::iterator iter = moduleIdMap.find(module);
    if (iter != moduleIdMap.end()){
        return iter->second;
    } else {
        cout << "***Tracer Error*** Module " << module->name() << " is not registered.";
        sc_stop();
        return 0; 
    }
}

#pragma mark -
#pragma mark Event Type Methods
int Tracer::registerEventType(EventType * eType){
    int index = ++eventTypeCount;
    outfile << "<eventtype id=\"E" << index << "\" name=\"" << eType->getName() << "\">\n";
    writeProperties(eType->getProperties());
    outfile << "</eventtype>\n";
    eventTypeIdMap[eType] = index;
    strEventTypeMap[eType->getFullName()] = eType;
    return index;
}

EventType* Tracer::getStrEventType(string name){
    EventType* et = strEventTypeMap[name];
    if (et == 0){
        et = new EventType(name);
        registerEventType(et);
    }
    return et;
}

int Tracer::getEventTypeId(EventType* eType){
    map<EventType *, int>::iterator iter = eventTypeIdMap.find(eType);
    if (iter != eventTypeIdMap.end()){
        return iter->second;
    } else {
        return registerEventType(eType);
    }
}

#pragma mark -
#pragma mark Trace Methods
int Tracer::registerTrace(Trace *trace){
    //TODO: Check that trace name is unique
    int index = ++traceCount;
    if (trace->getName() != "")
        outfile << "<trace id=\"T" << index << "\" name=\"" << trace->getName() << "\">\n";
    else 
        outfile << "<trace id=\"T" << index << "\" name=\"T" << index << "\">\n";
    writeProperties(trace->getProperties());
    outfile << "</trace>\n";
    traceIdMap[trace] = index;
    return index;
}

void Tracer::retireTrace(Trace *trans){
    traceIdMap.erase(trans);
}

void Tracer::initializeTrace(Trace *trace){
    if (!initComplete) initialize();
    registerTrace(trace);
}

int Tracer::getTraceId(Trace* trans){
    map<Trace *, int>::iterator iter = traceIdMap.find(trans);
    if (iter != traceIdMap.end()){
        return iter->second;
    } else {
        return registerTrace(trans);
    }
}

#pragma mark -
#pragma mark TLM Payload Trace Methods
int Tracer::registerTrace(tlm_generic_payload *trans){
    int index = ++traceCount;
    outfile << "<trace id=\"T" << index << "\" name=\"" << "T"<< index << "\">\n";
    writeTlmGenericPayloadTraceProperties(trans);
    outfile << "</trace>\n";
    tlmPayloadIdMap[trans] = index;
    return index;
}

void Tracer::retireTrace(tlm_generic_payload *trans){
    tlmPayloadIdMap.erase(trans);
}

void Tracer::initializeTrace(tlm_generic_payload *trans){
    if (!initComplete) initialize();
    registerTrace(trans);
}

void Tracer::initializeTrace(string name, tlm_generic_payload *trans){
    if (!initComplete) initialize();
    int index = ++traceCount;
    outfile << "<trace id=\"T" << index << "\" name=\"" << name << "\">\n";
    writeTlmGenericPayloadTraceProperties(trans);
    outfile << "</trace>\n";
    //TODO: Check that name is unique and warning the user about the name map override if not
    tlmPayloadIdMap[trans] = index;
}

void Tracer::writeTlmGenericPayloadTraceProperties(tlm_generic_payload *trans){
    string command;
    switch (trans->get_command()){
        case tlm::TLM_READ_COMMAND: command = "Read"; break;
        case tlm::TLM_WRITE_COMMAND: command = "Write"; break;
        case tlm::TLM_IGNORE_COMMAND: command = "Ignore"; break;
    }
    outfile << "<property name=\"Command\" value=\"" << command << "\"/>\n";
    char buffer[1024];
    sprintf(buffer, "%X", trans->get_address());
    outfile << "<property name=\"Adress\" value=\"0x" << buffer << "\"/>\n";
    outfile << "<property name=\"Data Length\" value=\"" << trans->get_data_length() << "\"/>\n";
    outfile << "<property name=\"Streaming Width\" value=\"" << trans->get_streaming_width() << "\"/>\n";
    if (trans->get_byte_enable_ptr()){
        outfile << "<property name=\"Byte Enable Length\" value=\"" << trans->get_byte_enable_length() << "\"/>\n";
        outfile << "<property name=\"Byte Enable\" value=\"" << trans->get_byte_enable_ptr() << "\"/>\n";
    }
}

int Tracer::getTlmGenericPayloadId(tlm_generic_payload* trans){
    map<tlm_generic_payload *, int>::iterator iter = tlmPayloadIdMap.find(trans);
    if (iter != tlmPayloadIdMap.end()){
        return iter->second;
    } else {
        return registerTrace(trans);
    }
}

#pragma mark -
#pragma mark Event Methods for "tlm_generic_payload" objects

void Tracer::mark(sc_module *module, tlm_generic_payload *trans, sc_time &time, EventType *etype){
#ifndef LPTRACE_OFF
    int traceId, moduleId, eventTypeId;
    if (!initComplete) initialize();
    traceId = getTlmGenericPayloadId(trans);
    moduleId = getModuleId(module);
    eventTypeId = getEventTypeId(etype);
    char buffer[1024];
    sprintf(buffer, "%g", time.to_seconds());
    outfile << "<event type=\"E" << eventTypeId << "\" trace=\"T" << traceId << "\" module=\"M" << moduleId << "\" time=\"" << buffer << "\">\n";
    writeTlmGenericPayloadEventProperties(trans);
    outfile << "</event>\n";    
#endif
}

void Tracer::mark(sc_module *module, tlm_generic_payload *trans, EventType *etype){
    sc_time time = sc_time_stamp();
    Tracer::mark(module, trans, time, etype);
}

void Tracer::mark(sc_module *module, tlm_generic_payload *trans, sc_time &time, string eventType){
    if (!initComplete) initialize();  //Need to initialize here if needed sing getStrEventType writes to file
    Tracer::mark(module, trans, time, getStrEventType(eventType));
}

void Tracer::mark(sc_module *module, tlm_generic_payload *trans, string eventType){
    sc_time time = sc_time_stamp();
    Tracer::mark(module, trans, time, eventType);
}

void Tracer::mark(sc_module *module, tlm_generic_payload *trans, sc_time &time, EventType *etype, map<string, string> properties){
#ifndef LPTRACE_OFF
    int traceId, moduleId, eventTypeId;
    if (!initComplete) initialize();
    traceId = getTlmGenericPayloadId(trans);
    moduleId = getModuleId(module);
    eventTypeId = getEventTypeId(etype);
    char buffer[1024];
    sprintf(buffer, "%g", time.to_seconds());
    outfile << "<event type=\"E" << eventTypeId << "\" trace=\"T" << traceId << "\" module=\"M" << moduleId << "\" time=\"" << buffer << "\">\n";
    writeTlmGenericPayloadEventProperties(trans);
    writeProperties(&properties);
    outfile << "</event>\n";    
#endif
}

void Tracer::mark(sc_module *module, tlm_generic_payload *trans, EventType *etype, map<string, string> properties){
    sc_time time = sc_time_stamp();
    Tracer::mark(module, trans, time, etype, properties);
}

void Tracer::mark(sc_module *module, tlm_generic_payload *trans, sc_time &time, string eventType, map<string, string> properties){
    if (!initComplete) initialize();  //Need to initialize here if needed sing getStrEventType writes to file
    Tracer::mark(module, trans, time, getStrEventType(eventType), properties);
}

void Tracer::mark(sc_module *module, tlm_generic_payload *trans, string eventType, map<string, string> properties){
    sc_time time = sc_time_stamp();
    Tracer::mark(module, trans, time, eventType, properties);
}

void Tracer::writeTlmGenericPayloadEventProperties(tlm_generic_payload *trans){
    outfile << "<property name=\"Response Status\" value=\"" << trans->get_response_string() << "\"/>\n";
    unsigned char *ptr = trans->get_data_ptr();
    if (ptr){
        outfile << "<property name=\"Data\" value=\"0x";
        for (int i = trans->get_data_length()-1; i >= 0; i--){
            outfile << std::setw( 2 ) << std::setfill('0') << std::uppercase << hex << (int)ptr[i];
        }
        outfile << "\"/>\n";
    }
}


#pragma mark -
#pragma mark Event Methods for "Trace" objects

void Tracer::mark(sc_module *module, Trace *trans, sc_time &time, EventType *etype){
#ifndef LPTRACE_OFF
    int traceId, moduleId, eventTypeId;
    if (!initComplete) initialize();
    traceId = getTraceId(trans);
    moduleId = getModuleId(module);
    eventTypeId = getEventTypeId(etype);
    char buffer[1024];
    sprintf(buffer, "%g", time.to_seconds());
    outfile << "<event type=\"E" << eventTypeId << "\" trace=\"T" << traceId << "\" module=\"M" << moduleId << "\" time=\"" << buffer << "\"/>\n";
#endif
}   

void Tracer::mark(sc_module *module, Trace *trans, EventType *etype){
    sc_time time = sc_time_stamp();
    Tracer::mark(module, trans, time, etype);
}

void Tracer::mark(sc_module *module, Trace *trans, sc_time &time, string eventType){
    if (!initComplete) initialize();  //Need to initialize here if needed sing getStrEventType writes to file
    Tracer::mark(module, trans, time, getStrEventType(eventType));
}

void Tracer::mark(sc_module *module, Trace *trans, string eventType){
    sc_time time = sc_time_stamp();
    Tracer::mark(module, trans, time, eventType);
}

void Tracer::mark(sc_module *module, Trace *trans, sc_time &time, EventType *etype, map<string, string> properties){
#ifndef LPTRACE_OFF
    int traceId, moduleId, eventTypeId;
    if (!initComplete) initialize();
    traceId = getTraceId(trans);
    moduleId = getModuleId(module);
    eventTypeId = getEventTypeId(etype);
    char buffer[1024];
    sprintf(buffer, "%g", time.to_seconds());
    outfile << "<event type=\"E" << eventTypeId << "\" trace=\"T" << traceId << "\" module=\"M" << moduleId << "\" time=\"" << buffer << "\">\n";
    writeProperties(&properties);
    outfile << "</event>\n";  
#endif
}

void Tracer::mark(sc_module *module, Trace *trans, EventType *etype, map<string, string> properties){
    sc_time time = sc_time_stamp();
    Tracer::mark(module, trans, time, etype, properties);
}

void Tracer::mark(sc_module *module, Trace *trans, sc_time &time, string eventType, map<string, string> properties){
    if (!initComplete) initialize();  //Need to initialize here if needed sing getStrEventType writes to file
    Tracer::mark(module, trans, time, getStrEventType(eventType), properties);
}

void Tracer::mark(sc_module *module, Trace *trans, string eventType, map<string, string> properties){
    sc_time time = sc_time_stamp();
    Tracer::mark(module, trans, time, eventType, properties);
}

#pragma mark -
#pragma mark Misc Methods
void Tracer::writeProperties(map<string,string> *props){
    map<string,string>::iterator iter;
    iter = props->begin();
    while(iter != props->end()){
        outfile << "<property name=\"" << iter->first << "\" value=\"" << iter->second << "\"/>\n";
        iter++;
    }
}

#pragma mark -
#pragma mark sc_trace_file methods
void Tracer::trace(const bool&, const std::string&){}
void Tracer::trace(const sc_dt::sc_bit&, const std::string&){}
void Tracer::trace(const sc_dt::sc_logic&, const std::string&){}
void Tracer::trace(const unsigned char&, const std::string&, int){}
void Tracer::trace(const short unsigned int&, const std::string&, int){}
void Tracer::trace(const unsigned int&, const std::string&, int){}
void Tracer::trace(const long unsigned int&, const std::string&, int){}
void Tracer::trace(const char&, const std::string&, int){}
void Tracer::trace(const short int&, const std::string&, int){}
void Tracer::trace(const int&, const std::string&, int){}
void Tracer::trace(const long int&, const std::string&, int){}
void Tracer::trace(const sc_dt::int64&, const std::string&, int){}
void Tracer::trace(const sc_dt::uint64&, const std::string&, int){}
void Tracer::trace(const float&, const std::string&){}
void Tracer::trace(const double&, const std::string&){}
void Tracer::trace(const sc_dt::sc_int_base&, const std::string&){}
void Tracer::trace(const sc_dt::sc_uint_base&, const std::string&){}
void Tracer::trace(const sc_dt::sc_signed&, const std::string&){}
void Tracer::trace(const sc_dt::sc_unsigned&, const std::string&){}
void Tracer::trace(const sc_dt::sc_fxval&, const std::string&){}
void Tracer::trace(const sc_dt::sc_fxval_fast&, const std::string&){}
void Tracer::trace(const sc_dt::sc_fxnum&, const std::string&){}
void Tracer::trace(const sc_dt::sc_fxnum_fast&, const std::string&){}
void Tracer::trace(const sc_dt::sc_bv_base&, const std::string&){}
void Tracer::trace(const sc_dt::sc_lv_base&, const std::string&){}
void Tracer::trace(const unsigned int&, const std::string&, const char**){}
void Tracer::write_comment(const std::string&){}
void Tracer::cycle(bool){}






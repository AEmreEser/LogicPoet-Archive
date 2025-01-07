/*
 *  Tracer.h
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

#ifndef _LPT_TRACER_H_
#define _LPT_TRACER_H_

#include "systemc.h"
#include "tlm.h"
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include "Trace.h"
#include "EventType.h"

using sc_core::sc_module;
using sc_core::sc_time;
using tlm::tlm_generic_payload;
using std::string;
using std::vector;
using std::map;
using namespace lpt;
using sc_core::sc_trace_file;

#pragma mark -
#pragma mark Tracer Global Defines
//Global defines for easy use of the shared tracer.  Marking events provides the 
//pointer to the source module but can only be called from within an sc_module
#define MarkEvent(...) Tracer::getSharedTracer()->mark(this,##__VA_ARGS__);
#define InitializeTrace(...) Tracer::getSharedTracer()->initializeTrace(##__VA_ARGS__);
#define RetireTrace(trace) Tracer::getSharedTracer()->retireTrace(trace);

#pragma mark -

namespace lpt{

    class Tracer : public sc_trace_file{
    public:
#pragma mark -
#pragma mark Initializers & Destructors
        //Ideally this gets used as a singleton, but the constructors are public in case
        //you need to have multiple trace files for some reason.  You can't use the defined 
        //macros in that case though.
        Tracer::Tracer();
        Tracer(char * filename);
        static Tracer* getSharedTracer();
        ~Tracer();
#pragma mark -
#pragma mark TLM OSCI Payload Recording
        void mark(sc_module *module, tlm_generic_payload *trans, sc_time &time, EventType *etype);
        void mark(sc_module *module, tlm_generic_payload *trans, EventType *etype);
        void mark(sc_module *module, tlm_generic_payload *trans, sc_time &time, string eventType);
        void mark(sc_module *module, tlm_generic_payload *trans, string eventType);
        void mark(sc_module *module, tlm_generic_payload *trans, sc_time &time, EventType *etype, map<string, string> properties);
        void mark(sc_module *module, tlm_generic_payload *trans, EventType *etype, map<string, string> properties);
        void mark(sc_module *module, tlm_generic_payload *trans, sc_time &time, string eventType, map<string, string> properties);
        void mark(sc_module *module, tlm_generic_payload *trans, string eventType, map<string, string> properties);
        void initializeTrace(string name, tlm_generic_payload *trans);
        void initializeTrace(tlm_generic_payload *trans);
        void retireTrace(tlm_generic_payload *trans);
#pragma mark -
#pragma mark Custom Trace Recording
        void mark(sc_module *module, Trace *trans, sc_time &time, EventType *etype);
        void mark(sc_module *module, Trace *trans, EventType *etype);
        void mark(sc_module *module, Trace *trans, sc_time &time, string eventType);
        void mark(sc_module *module, Trace *trans, string eventType);
        void mark(sc_module *module, Trace *trans, sc_time &time, EventType *etype, map<string, string> properties);
        void mark(sc_module *module, Trace *trans, EventType *etype, map<string, string> properties);
        void mark(sc_module *module, Trace *trans, sc_time &time, string eventType, map<string, string> properties);
        void mark(sc_module *module, Trace *trans, string eventType, map<string, string> properties);
        void initializeTrace(Trace *trace);
        //This must be called when re-using a trace pointer for a new trace
        void retireTrace(Trace *trace);     
#pragma mark -
#pragma mark sc_trace_file methods
        //Methods required for sc_trace_file inheritance.  Not implemeted yet, but 
        //in a future revision this will be use for tracing waves as well as TLM events
        void trace(const bool&, const std::string&);
        void trace(const sc_dt::sc_bit&, const std::string&);
        void trace(const sc_dt::sc_logic&, const std::string&);
        void trace(const unsigned char&, const std::string&, int);
        void trace(const short unsigned int&, const std::string&, int);
        void trace(const unsigned int&, const std::string&, int);
        void trace(const long unsigned int&, const std::string&, int);
        void trace(const char&, const std::string&, int);
        void trace(const short int&, const std::string&, int);
        void trace(const int&, const std::string&, int);
        void trace(const long int&, const std::string&, int);
        void trace(const sc_dt::int64&, const std::string&, int);
        void trace(const sc_dt::uint64&, const std::string&, int);
        void trace(const float&, const std::string&);
        void trace(const double&, const std::string&);
        void trace(const sc_dt::sc_int_base&, const std::string&);
        void trace(const sc_dt::sc_uint_base&, const std::string&);
        void trace(const sc_dt::sc_signed&, const std::string&);
        void trace(const sc_dt::sc_unsigned&, const std::string&);
        void trace(const sc_dt::sc_fxval&, const std::string&);
        void trace(const sc_dt::sc_fxval_fast&, const std::string&);
        void trace(const sc_dt::sc_fxnum&, const std::string&);
        void trace(const sc_dt::sc_fxnum_fast&, const std::string&);
        void trace(const sc_dt::sc_bv_base&, const std::string&);
        void trace(const sc_dt::sc_lv_base&, const std::string&);
        void trace(const unsigned int&, const std::string&, const char**);
        void write_comment(const std::string&);
        void cycle(bool);
#pragma mark -
#pragma mark Filename Accessors
        //Note: filenames cannot be set after the first event is marked
        void setFilename(string filename);
        static void setSharedFilename(string filename);
        string getFilename();
        static string getSharedFilename();
    protected:
#pragma mark -
#pragma mark Internal Methods
        //Constructor & Singleton Handling
        void initialize();
        string filename;
        //File Management
        ofstream outfile;
        bool initComplete;
        //Modules
        int moduleCount;
        map<sc_module *, int> moduleIdMap;
        void registerAllModules();
        void registerModule(sc_object* mod);
        int getModuleId(sc_module* module);
        //Event Types
        int eventTypeCount;
        map<EventType *, int> eventTypeIdMap;
        map<string, EventType *> strEventTypeMap;
        int registerEventType(EventType * eType);
        EventType* getStrEventType(string name);
        int getEventTypeId(EventType* eType);
        //Traces
        int traceCount;
        map<Trace *, int> traceIdMap;
        int registerTrace(Trace *trace);
        int getTraceId(Trace* trans);
        void writeProperties(map<string, string> *props);
        //tlm_generic_payload 
        map<tlm_generic_payload *, int> tlmPayloadIdMap;
        int registerTrace(tlm_generic_payload *trans);
        int getTlmGenericPayloadId(tlm_generic_payload* trans);
        void writeTlmGenericPayloadTraceProperties(tlm_generic_payload *trans);
        void writeTlmGenericPayloadEventProperties(tlm_generic_payload *trans);
    };  

} //namespace lpt

#endif
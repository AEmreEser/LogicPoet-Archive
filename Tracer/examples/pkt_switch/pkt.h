/*****************************************************************************

  The following code is derived, directly or indirectly, from the SystemC
  source code Copyright (c) 1996-2006 by all Contributors.
  All Rights reserved.

  The contents of this file are subject to the restrictions and limitations
  set forth in the SystemC Open Source License Version 2.4 (the "License");
  You may not use this file except in compliance with such restrictions and
  limitations. You may obtain instructions on how to receive a copy of the
  License at http://www.systemc.org/. Software distributed by Contributors
  under the License is distributed on an "AS IS" basis, WITHOUT WARRANTY OF
  ANY KIND, either express or implied. See the License for the specific
  language governing rights and limitations under the License.

 *****************************************************************************/

/*****************************************************************************

  pkt.h - This file defines a SystemC structure "pkt".

  Original Author: Rashmi Goswami, Synopsys, Inc.

 *****************************************************************************/

/*****************************************************************************

  MODIFICATION LOG - modifiers, enter your name, affiliation, date and
  changes you are making here.

      Name, Affiliation, Date:
  Description of Modification:

 *****************************************************************************/

#include "systemc.h"
//LOGICPOET:Added an lpt::Trace member and included the header for the tracer
//          Note that this is being done via composition only because this sim
//          is designed to pass packets by value.  Had it been done by passing 
//          pointers (which is more efficient anyway) this would have been simply
//          done by adding Trace to the public ingeritance of the pkt class and 
//          overriding the getProperties() method of Trace to return the properties.
#include "lptracer/Tracer.h"
struct pkt{
       sc_int<8> data;
       sc_int<4> id;
       bool dest0;
       bool dest1;
       bool dest2;
       bool dest3;

    //LOGICPOET:Added a trace pointer member
    Trace *trace;
    
    //LOGICPOET:Added method to get properties in a map to pass to the tracer
    map<string,string> getProperties() {
        char buffer[8];
        map<string,string> properties;
        sprintf(buffer, "%d", id.to_int());
        properties["Sender"] = string(buffer);
        properties["Targets Receiver 0"] = dest0 ? "True" : "False";
        properties["Targets Receiver 1"] = dest1 ? "True" : "False";
        properties["Targets Receiver 2"] = dest2 ? "True" : "False";
        properties["Targets Receiver 3"] = dest3 ? "True" : "False";
        sprintf(buffer, "%X", data.to_int());
        properties["Data"] = "0x"+string(buffer);
        return properties;
    }
    
    
    inline bool operator == (const pkt& rhs) const
    {
        return (rhs.data == data && rhs.id == id && rhs.dest0 == dest0 && rhs.dest1 == dest1 && rhs.dest2 == dest2 && rhs.dest3 == dest3);
    }

};

inline
ostream&
operator << ( ostream& os, const pkt& a )
{
    os << "streaming of struct pkt not implemented";
    return os;
}

inline
void
#if defined(SC_API_VERSION_STRING)
    sc_trace( sc_trace_file* tf, const pkt& a, const std::string& name )
#else
    sc_trace( sc_trace_file* tf, const pkt& a, const sc_string& name )
#endif
{
  sc_trace( tf, a.data, name + ".data" );
  sc_trace( tf, a.id, name + ".id" );
  sc_trace( tf, a.dest0, name + ".dest0" );
  sc_trace( tf, a.dest1, name + ".dest1" );
  sc_trace( tf, a.dest2, name + ".dest2" );
  sc_trace( tf, a.dest3, name + ".dest3" );
}

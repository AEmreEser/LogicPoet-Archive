Logic Poet
www.logicpoet.com

Tracer

*** Overview ***
The Tracer library is an open source tool for logging events in a SystemC simulation.  The current implementation supports logging transaction level traces.  It has tracing routines for both OSCI's TLM2 tlm_generic_payload objects, and also for a custom Trace object for non-TLM2 applications.  The Tracer generates an XML file consistent with the DTD located at http://www.logicpoet.com/DTD/scansion.dtd.  This allows it to be directly visualized and analyzed in Scansion (for more details on Scansion visit http://www.logicpoet.com/scansion), though since it is an open XML format you can easily write other tools that can read this output.

The transaction logging is done by marking events onto a trace of a particular transaction as it moves through the system.  For example, you might have a CPU writing to memory as the transaction with events marked along the way such as gaining bus ownership, entering arbitration to memory, winning arbitration to memory, and completing the operation in memory.  You are free to choose the events that are useful to you, so you can make these as high level or low level as you like.  

Tracer defines the following classed in namespace lpt:
	Tracer:		The main class for handling event recording and file output
	Trace:		Used for defining custom trace packets upon which events can be marked and properties can be assigned.
	EventType:	Defines classes of events that can be recorded on a trace.
	ModelBase:	Base class for Trace and EventType that currently just has a property map member and some accessors for assigning properties.

*** General Use ***
Assuming you are using the Logic Poet SystemC installer, you only need to include the header "lptracer/Tracer.h" in your source code where you want to do the event recording.  If you are not using the TLM2 payload, you will need to customize the packet type you are interested in tracing (more on that later).  Once that has been done you just need to mark the events you are interested in.  

In the simplest case, you just record the event type as a descriptive string and Tracer takes care of the rest.  For example, if I had a pointer to a traceable packet (inheriting from Trace or from tlm_generic_payload) called "mypkt" I could simply issue:
	MarkEvent(mypkt, "An Interesting Event");
The first time this is called on any instance of mypkt an event will be created called "An Interesting Event".  You will be able to see this in Scansion.

You can also create predefined event types via the EventType class and pass that to the MarkEvent routine instead.  This is useful if you want to create a set of events for a well defined interface or system and make that used by many agents.  This is preferable vs using the descriptive string when you have a large project with many people working on it since it ensures that all are using the same event types and avoids errors when typing the string.  A good way to do this is to create a container class that returns singleton EventType objects for each interesting event.  

Where marking events on a trace there are several options you can chose to use or omit.  In general Tracer will handle the recording of the simulation time when an event is marked.  However if you need to specify a different time you can do so.  Also there is an option to add a map object containing a property/value list for the event which you can use for anything you like.  The key for the map is the name of the property and the value is the string value associated with that property.  The propertied will be viewable in Scansion when the event is selected.  Note that the tlm_generic_payload properties are handed by the Tracer so there is no need to explicitly add these.

Note that, like events, Trace objects can also have properties assigned.  This is a better way to assign properties that don't change throughout the life of the trace (like the address of a request for example) instead of recording them on every event since it unnecessarily increases the file size of the trace file.

The Tracer API by default is done using global macros to operate on a singleton instance of the Tracer.  It is possible to use multiple instances as well, but you can't use the macros for this so check out the source code if you are interested in this.

One important thing to note is that Tracer holds the trace pointer (a pointer to either a tlm_generic_payload or a Trace class) after the first event is recorded on it and assumes this is the same trace when another event is recorded on it.  This is problematic if the pointer is being reused for a new transaction after the old one completes, since it would appear to be the same trace to the Tracer rather than the new trace that it is.  To avoid this issue it is important retire the trace before reusing.  This can be done by calling the RetireTrace(...) macro which is described below.

Related to this, to avoid memory leaks and potential unintended aliasing, it is important to retire your traces when you are done with them.  Tracer keeps a record of the pointers for all TRace or tlm_generic_payload object that have been initialized or had events marked on them.  calling the RetireTrace(...) macro will allow the Tracer to know that the life of that event is finished and it can remove it from its records.

The name of a trace can be assigned by using the InitializeTrace(string name, tlm_generic_payload *trans) or InitializeTrace(string name, Trace *trans) macro before the first event is marked on it.  If this is not done, the trace gets a default name based on the number of traces used so far (if it is the 5th trace that the Tracer knows about when its first event is recorded it will be called "T5").  Note that this initialization is optional.

*** Use With OSCI TLM2.0 tlm_generic_payload ***
Tracer handles most of the busy work in recording the details of a TLM trace.  When you tracer a tlm_generic_payload object, the properties of that object will automatically get traced for later viewing.  All you need to do is mark the event as shown above.  

The following routines are defined in Tracer.h and available for recording events for tlm_generic_payload objects:
		MarkEvent(tlm_generic_payload *trans, sc_time &time, EventType *etype);
		MarkEvent(tlm_generic_payload *trans, sc_time &time, EventType *etype);
		MarkEvent(tlm_generic_payload *trans, EventType *etype);
		MarkEvent(tlm_generic_payload *trans, sc_time &time, string eventType);
		MarkEvent(tlm_generic_payload *trans, string eventType);
		MarkEvent(tlm_generic_payload *trans, sc_time &time, EventType *etype, map<string, string> properties);
		MarkEvent(tlm_generic_payload *trans, EventType *etype, map<string, string> properties);
		MarkEvent(tlm_generic_payload *trans, sc_time &time, string eventType, map<string, string> properties);
		MarkEvent(tlm_generic_payload *trans, string eventType, map<string, string> properties);
		
The following routines are defined in Tracer.h and available for retiring or initializing tlm_generic_payload trace recording:
		InitializeTrace(string name, tlm_generic_payload *trans);
		InitializeTrace(tlm_generic_payload *trans);
		RetireTrace(tlm_generic_payload *trans);
		

*** Use With Custom non-TLM2 Packets ***
If you have a system that is not using the tlm_generic_payload you can easily trace it as well, it just takes a couple of more steps.  There are two general options for modifying a packet for tracing: you can add a Trace object to a packet as a member or you can have the packet inherit from Trace (publicly).  

The composition route of adding a Trace member to your packet is necessary when packets are passed around by value.  Remember the Tracer uses pointers to Trace objects as the identity of a trace so this cannot be used in a simulation that uses a pass-by-value model for packet transfer.  when using this, remember that the event marking is going to be done on the Trace member of your packet (i.e. MarkEvent(mypkt.trace, "Some Event"); ).  You will also need to add properties to the Trace member manually.  See the pkt_switch example provided (likely located at /Library/SystemC/examples/Tracer/pkt_switch) for one approach to doing this.  

Inheritance is a simpler route if you can be assured that all transfers are doing by passing pointers.  Add lpt::Trace as a public parent of the class you would like to trace and you are most of the way done.  A convenient addition is to override the getProperties() method from the Trace class to return a map<string,string> object with the property list that describes the packet's members that will be constant throughout the life of the packet (like address, byte enable, source, etc).

One other note.  Unlike tlm_generic_payload, Trace has a "name" member, and assigning this is the right/only way to assign a name to a Trace object.  If this is left blank, Tracer will provide a name similar to when a tlm_generic_payload trace is not initialized (see above).  There is no InitializeTrace macro for assigning this a name, unlike for tlm_generic_payload.

The following routines are defined in Tracer.h and available for recording events for Trace objects:
		MarkEvent(Trace *trans, sc_time &time, EventType *etype);
		MarkEvent(Trace *trans, sc_time &time, EventType *etype);
		MarkEvent(Trace *trans, EventType *etype);
		MarkEvent(Trace *trans, sc_time &time, string eventType);
		MarkEvent(Trace *trans, string eventType);
		MarkEvent(Trace *trans, sc_time &time, EventType *etype, map<string, string> properties);
		MarkEvent(Trace *trans, EventType *etype, map<string, string> properties);
		MarkEvent(Trace *trans, sc_time &time, string eventType, map<string, string> properties);
		MarkEvent(Trace *trans, string eventType, map<string, string> properties);
		
The following routines are defined in Tracer.h and available for retiring or initializing tlm_generic_payload trace recording:
		InitializeTrace(Trace *trans);
		RetireTrace(Trace *trans);


*** Output File Name ***
Like most of the rest of Tracer, there is flexibility there if you want it but you don't have to use it.  By default the transaction trace details will be recorded to a file called "tracefile.scnx" in the directory where the simulation is being run.  If you would like to specify your own filename you can do so.  If you are using the default (singleton) Tracer object you can specify the filename via the static method: setSharedFilename(string, name).  For example: lpt::Tracer::setSharedFilename("myfile.scnx"); to set the filename to "myfile.scnx".  If you don't finish the string with ".scnx", which is the extension for Scansion XML files, that extension will get appended to the name you specify.  

*** Other notes ***
If you are looking at the source code, you may be wondering why Tracer inherits from sc_core::sc_trace_file.  There is no functionality there yet, but this class is preparing to be used as a unified tracing utility for both transactions and waveforms.  Stay tuned.

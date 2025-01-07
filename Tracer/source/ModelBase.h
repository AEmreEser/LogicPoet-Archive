/*
 *  ModelBase.h
 *  LPTracer
 *
 *  http://www.logicpoet.com
 *
 *  Created by Jeff Wilcox on 6/29/08.
 *  Copyright 2008 Logic Poet.  
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

#ifndef _LPT_MODEL_BASE_H_
#define _LPT_MODEL_BASE_H_

#include <string>
#include <map>

using std::string;
using std::map;

namespace lpt{

class ModelBase{
protected:
    map<string,string> properties;
public:
    map<string,string>* getProperties() { return &properties; }
    void setProperties(map<string,string> propertyMap){
        this->properties = propertyMap;
    }
    void addProperty(string name, string value){
        properties[name] = value;
    }
    void addProperty(string name, int  value){
        char buffer[128];
        sprintf(buffer, "%d", value);
        addProperty(name, buffer);
    }
    void addProperty(string name, float  value){
        char buffer[128];
        sprintf(buffer, "%g", value);
        addProperty(name, buffer);
    }
    void addProperty(string name, double  value){
        char buffer[128];
        sprintf(buffer, "%g", value);
        addProperty(name, buffer);
    }
    void addProperty(string name, bool  value){
        string strValue = (value) ? "True" : "False";
        addProperty(name, strValue);
    }
};
    
}  //namespace lpt


#endif
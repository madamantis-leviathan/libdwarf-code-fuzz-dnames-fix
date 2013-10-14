/*
  Copyright (C) 2010-2013 David Anderson.  All rights reserved.
 
  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:
  * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
  * Neither the name of the example nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.
 
  THIS SOFTWARE IS PROVIDED BY David Anderson ''AS IS'' AND ANY
  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL David Anderson BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/


// 
// irepdie.h
//
//
class IRCUdata;

class IRAttr { 
public:
    IRAttr():attr_(0),finalform_(0),initialform_(0),
        formclass_(DW_FORM_CLASS_UNKNOWN),formdata_(0) {
        };
    IRAttr(Dwarf_Half attr,Dwarf_Half finalform, Dwarf_Half initialform):
        attr_(attr),finalform_(finalform),initialform_(initialform),
        formclass_(DW_FORM_CLASS_UNKNOWN),formdata_(0) {
    };
    IRAttr(const IRAttr &r) {
        attr_ = r.attr_;
        finalform_ = r.finalform_;
        initialform_ = r.initialform_;
        formclass_ = r.formclass_;
        if(r.formdata_) {
            formdata_ =  r.formdata_->clone();
        } else {
            formdata_ = 0;
        }
    };
    ~IRAttr() { delete formdata_; };
    IRAttr & operator=( const IRAttr &r) {
        if(this == &r) {
            return *this;
        }
        attr_ = r.attr_;
        finalform_ = r.finalform_;
        initialform_ = r.initialform_;
        formclass_ = r.formclass_;
        if(r.formdata_) {
            formdata_ =  r.formdata_->clone();
        } else {
            formdata_ = 0;
        }
        return *this;
    }
    void setBaseData(Dwarf_Half attr, Dwarf_Half finalform, 
        Dwarf_Half initialform){
        attr_ = attr;
        finalform_ = finalform;
        initialform_ = initialform;
    };
    void setFormClass(enum Dwarf_Form_Class cl) {
        formclass_ = cl;
    };
    enum Dwarf_Form_Class getFormClass() const {return formclass_; };
    void setFormData(IRForm *f) { formdata_ = f; };
    Dwarf_Half getFinalForm() const { return initialform_; };
    Dwarf_Half getDirectForm() const { return finalform_; };
    Dwarf_Half getAttrNum() const { return attr_; };
    IRForm * getFormData() { return formdata_;};
private:
    Dwarf_Half attr_;
    Dwarf_Half finalform_;
    // In most cases finalform == initialform form.
    // Otherwise, initialform == DW_FORM_indirect.
    Dwarf_Half initialform_;
    enum Dwarf_Form_Class formclass_;
    IRForm *formdata_;
};

class IRDie {
public:
    IRDie():tag_(0),globalOffset_(0), cuRelativeOffset_(0) {};
    ~IRDie() {};
    void addChild(const IRDie & newdie ) {
        children_.push_back(newdie);
    };
    std::string  getName() {
        std::list<IRAttr>::iterator it = attrs_.begin(); 
        for( ; it != attrs_.end() ; ++it) {
            if (it->getAttrNum() == DW_AT_name) {
                IRForm *f = it->getFormData();
                const IRFormString * isv = 
                    dynamic_cast<const IRFormString *>(f);
                if(isv) {
                    return isv->getString();
                }
            }
        }
        return "";
    };
    std::list<IRAttr> & getAttributes() {return attrs_; };
    std::list<IRDie> & getChildren() {return children_; };
    bool hasNewestChild(IRDie **lastch) { size_t N = children_.size();
        if(N < 1) {
            return false;
        }
        *lastch = &children_.back();
        return true;
    };
    // lastChild will throw if no child exists.
    IRDie &lastChild() { return children_.back(); };
    void setBaseData(Dwarf_Half tag,Dwarf_Unsigned goff, 
        Dwarf_Unsigned cuoff) {
        tag_ = tag;
        globalOffset_=goff;
        cuRelativeOffset_ = cuoff; 
    };
    Dwarf_Unsigned getGlobalOffset() const { return globalOffset_;};
    unsigned getTag() {return tag_; }

private:
   std::list<IRDie>  children_;
   std::list<IRAttr> attrs_;
   unsigned tag_;
   Dwarf_Unsigned globalOffset_;
   Dwarf_Unsigned cuRelativeOffset_;
};

class IRCUdata {
public:
    IRCUdata(): 
        cu_header_length_(0),
        abbrev_offset_(0),
        next_cu_header_offset_(0),
        version_stamp_(0),
        address_size_(0),
        length_size_(0),
        extension_size_(0),
        has_macrodata_(false),
        macrodata_offset_(0),
        has_linedata_(false),
        linedata_offset_(0),
        cudie_offset_(0)
        {};
    IRCUdata(Dwarf_Unsigned len,Dwarf_Half version,
        Dwarf_Unsigned abbrev_offset,
        Dwarf_Half addr_size,
        Dwarf_Half length_size,
        Dwarf_Half extension_size,
        Dwarf_Unsigned next_cu_header): 
            cu_header_length_(len),
            abbrev_offset_(abbrev_offset),
            next_cu_header_offset_(addr_size),
            version_stamp_(version),
            address_size_(addr_size),
            length_size_(length_size),
            extension_size_(extension_size),
            has_macrodata_(false),
            macrodata_offset_(0),
            has_linedata_(false),
            linedata_offset_(0),
            cudie_offset_(0) {};
    ~IRCUdata() { };
    bool hasMacroData(Dwarf_Unsigned *offset_out,Dwarf_Unsigned *cudie_off) {
        *offset_out = macrodata_offset_;
        *cudie_off = cudie_offset_; 
        return has_macrodata_;
    }
    bool hasLineData(Dwarf_Unsigned *offset_out,Dwarf_Unsigned *cudie_off) {
        *offset_out = linedata_offset_;
        *cudie_off = cudie_offset_; 
        return has_linedata_;
    }
    void setMacroData(Dwarf_Unsigned offset,Dwarf_Unsigned cudieoff) {
        has_macrodata_ = true;
        macrodata_offset_ = offset;
        cudie_offset_ = cudieoff;
    };
    void setLineData(Dwarf_Unsigned offset,Dwarf_Unsigned cudieoff) {
        has_linedata_ = true;
        linedata_offset_ = offset;
        cudie_offset_ = cudieoff;
    };
    IRDie & baseDie() { return cudie_; };
    Dwarf_Half getVersionStamp() { return version_stamp_; };
    Dwarf_Half getOffsetSize() { return length_size_; };
    IRCULineData & getCULines() { return cu_lines_; };
    std::string  getCUName() {
        return cudie_.getName();
    }
    
private:
    Dwarf_Unsigned cu_header_length_;
    Dwarf_Unsigned abbrev_offset_;
    Dwarf_Unsigned next_cu_header_offset_;
    Dwarf_Half  version_stamp_;
    Dwarf_Half address_size_;
    Dwarf_Half length_size_;
    Dwarf_Half extension_size_;
    bool has_macrodata_;
    Dwarf_Unsigned macrodata_offset_;
    bool has_linedata_;
    Dwarf_Unsigned linedata_offset_;
    Dwarf_Unsigned cudie_offset_;
    IRCULineData      cu_lines_;

    // If true, is 32bit dwarf,else 64bit. Gives the size of a reference.
    bool dwarf32bit_;  

    IRDie   cudie_;
};

class IRDInfo {
public:
   IRDInfo() {};
   ~IRDInfo() {};
   IRCUdata &lastCU() { return cudata_.back(); }
   std::list<IRCUdata>& getCUData() {return cudata_; };
private:
   std::list<IRCUdata>  cudata_;
};


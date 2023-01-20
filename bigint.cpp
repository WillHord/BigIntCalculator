#include <cstdlib>
#include <exception>
#include <stack>
#include <stdexcept>
using namespace std;

#include "bigint.h"

bigint::bigint (long that): uvalue (that), is_negative (that < 0) {
   DEBUGF ('~', this << " -> " << uvalue)
}

bigint::bigint (const ubigint& uvalue_, bool is_negative_):
                uvalue(uvalue_), is_negative(is_negative_) {
}

bigint::bigint (const string& that) {
   is_negative = that.size() > 0 and that[0] == '_';
   uvalue = ubigint (that.substr (is_negative ? 1 : 0));
}

bigint bigint::operator+ () const {
   return *this;
}

bigint bigint::operator- () const {
   return {uvalue, not is_negative};
}

bigint bigint::operator+ (const bigint& that) const {
   ubigint result;
   bool negative;
   if(that.is_negative == is_negative){
      result = {uvalue + that.uvalue};
      negative = is_negative;
   } else {
      if(uvalue > that.uvalue){
         result = {uvalue - that.uvalue};
         negative = is_negative;
      } else{
         result = {that.uvalue - uvalue};
         negative = that.is_negative;
      }
   }
   bigint toreturn {result, negative};
   if(toreturn.uvalue == ubigint {"0"}){
      negative = false;
   }
   return bigint {result, negative};
}

bigint bigint::operator- (const bigint& that) const {
   ubigint result;
   bool negative;
   if(is_negative == that.is_negative){
      result = {max(uvalue, that.uvalue) - min(that.uvalue, uvalue)};
      if(uvalue <= that.uvalue){
         negative = !that.is_negative;
      } else {
         uvalue > that.uvalue ? 
            negative=is_negative : negative=that.is_negative;
      }
   } else {
      result = {uvalue + that.uvalue};
      negative = is_negative;
   }
   bigint toreturn {result, negative};
   if(toreturn.uvalue == ubigint {"0"} ){
      negative = false;
   }
   return bigint {result, negative};
}

bigint bigint::operator* (const bigint& that) const {
   ubigint result;
   bool negative;
   if(is_negative == that.is_negative){
      negative = false;
   } else {
      negative = true;
   }
   result = {max(uvalue, that.uvalue) * min(that.uvalue, uvalue)};
   return {result, negative};
}

bigint bigint::operator/ (const bigint& that) const {
   bigint result {uvalue / that.uvalue};
   return result;
}

bigint bigint::operator% (const bigint& that) const {
   bigint result {uvalue % that.uvalue};
   return result;
}

bool bigint::operator== (const bigint& that) const {
   return is_negative == that.is_negative and uvalue == that.uvalue;
}

bool bigint::operator< (const bigint& that) const {
   if (is_negative != that.is_negative) return is_negative;
   return is_negative ? uvalue > that.uvalue 
                      : uvalue < that.uvalue;
}

void bigint::print() const {
   DEBUGF ('p', this << " -> " << *this);
   if (is_negative) cout << "-";
   uvalue.print();
}

ostream& operator<< (ostream& out, const bigint& that) {
   return out << "bigint(" << (that.is_negative ? "-" : "+")
              << "," << that.uvalue << ")";
}


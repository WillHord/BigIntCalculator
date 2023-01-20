#include <cctype>
#include <cstdlib>
#include <exception>
#include <stack>
#include <stdexcept>
#include <vector>
using namespace std;

#include "debug.h"
#include "ubigint.h"

ubigint::ubigint (unsigned long that): uvalue (that) {
   vector<uint8_t> temp;
   if(that == 0){
      temp.push_back(0);
   } else {
      while(that != 0){
         temp.push_back(that % 10);
         that = (that - (that % 10)) / 10;
      }
   }
   uvalue = temp;
}

ubigint::ubigint (const string& that): uvalue(0) {
   DEBUGF ('~', "that = \"" << that << "\"");
   vector<uint8_t> temp;
   for (auto rit=that.rbegin(); rit != that.rend(); rit++){
      if (not isdigit (*rit)) {
         throw invalid_argument ("ubigint::ubigint(" + that + ")");
      }
      temp.push_back(*rit - '0');
   }
   uvalue = temp;
}

ubigint ubigint::operator+ (const ubigint& that) const {
   DEBUGF ('u', *this << "+" << that);
   ubigint result;
   uint8_t temp;
   if(uvalue.size() > that.uvalue.size()){
      int i = 0;
      uint8_t carryover = 0;
      for (; i < static_cast<int>(that.uvalue.size()); i++){
         temp = that.uvalue[i] + uvalue[i] + carryover;
         carryover = temp / 10;
         if(temp >= 10){
            temp = temp % 10;
         }
         result.uvalue.push_back(temp);
      }
      for(; i < static_cast<int>(uvalue.size()); i++){
         temp = uvalue[i] + carryover;
         carryover = temp / 10;
         if(temp >= 10){
            temp = temp % 10;
         }
         result.uvalue.push_back(temp);
      }
      if(carryover != 0){
         result.uvalue.push_back(carryover);
      }
   } else {
      int i=0;
      uint8_t carryover = 0;
      for (; i < static_cast<int>(uvalue.size()); i++){
         temp = that.uvalue[i] + uvalue[i] + carryover;
         carryover = temp / 10;
         if(temp >= 10){
            temp = temp % 10;
         }
         result.uvalue.push_back(temp);
      }

      for(; i < static_cast<int>(that.uvalue.size()); i++){
         temp = that.uvalue[i] + carryover;
         carryover = temp / 10;
         if(temp >= 10){
            temp = temp % 10;
         }
         result.uvalue.push_back(temp);
      }
      if(carryover != 0){
         result.uvalue.push_back(carryover);
      }
   }
   DEBUGF ('u', result);
   while(result.uvalue.back() == 0 and result.uvalue.size() > 1){
      result.uvalue.pop_back();
   }
   return result;
}

ubigint ubigint::operator- (const ubigint& that) const {
   ubigint result;
   int i = 0;
   uint8_t carry = 0;
   uint8_t temp;
   for(; i< static_cast<int>(that.uvalue.size()); i++){
      if(uvalue[i] < that.uvalue[i]){
         temp = 10 + uvalue[i] - that.uvalue[i] - carry;
         result.uvalue.push_back(temp);
         carry = 1;
      } else {
         temp = uvalue[i] - that.uvalue[i];
         if(temp > 0){
            temp = temp - carry;
            carry = 0;
         } else if (carry > 0 ){
            temp = 10 + temp - carry;
            carry = 1;
         }
         result.uvalue.push_back(temp);
      }
   }
   for(; i< static_cast<int>(uvalue.size()); i++){
      if(uvalue[i] == 0 and carry != 0){
         result.uvalue.push_back(9);
         carry = 1;
      } else {
         result.uvalue.push_back(uvalue[i] - carry);
         carry = 0;
      }
   }
   while(result.uvalue.back() == 0 and result.uvalue.size() > 1){
      result.uvalue.pop_back();
   }
   return result;
}

ubigint ubigint::operator* (const ubigint& that) const {
   ubigint p;
   uint8_t digit, carry;
   vector<uint8_t> temp(static_cast<int>(that.uvalue.size()) +
      static_cast<int>(uvalue.size()),0);
   p.uvalue = temp;
   int i, j;
   for(i=0; i < static_cast<int>(uvalue.size()); i++){
      carry = 0;
      for(j=0; j < static_cast<int>(that.uvalue.size()); j++){
         digit = p.uvalue[i+j] + uvalue[i] * that.uvalue[j] + carry;
         p.uvalue[i+j] = digit % 10;
         carry = digit / 10;
      }
      p.uvalue[i+j] = carry;
   }
   while(p.uvalue.back() == 0 and p.uvalue.size() > 1)
      p.uvalue.pop_back();

   return p;
}

void ubigint::multiply_by_2() {
   uint8_t carry = 0;
   for(int i=0; i < static_cast<int>(uvalue.size()); i++){
      uvalue[i] = uvalue[i] * 2 + carry;
      carry = uvalue[i] / 10;
      if(uvalue[i] >= 10){
         uvalue[i] = uvalue[i] % 10;
      }
   }
   if(carry == 1){
      uvalue.push_back(1);
   }
}

void ubigint::divide_by_2() {
   for(int i=0; i < static_cast<int>(uvalue.size()); i++){
      uvalue[i] = uvalue[i] / 2;
      if(i < static_cast<int>(uvalue.size())-1 and 
         uvalue[i+1] % 2 != 0){
         uvalue[i] += 5;
      }
   }
   while(uvalue.back() == 0 and uvalue.size() > 1) uvalue.pop_back();
}


struct quo_rem { ubigint quotient; ubigint remainder; };
quo_rem udivide (const ubigint& dividend, const ubigint& divisor_) {
   // NOTE: udivide is a non-member function.
   ubigint divisor {divisor_};
   ubigint zero {0};
   if (divisor == zero) throw domain_error ("udivide by zero");
   ubigint power_of_2 {1};
   ubigint quotient {0};
   ubigint remainder {dividend}; // left operand, dividend
   while (divisor < remainder) {
      divisor.multiply_by_2();
      power_of_2.multiply_by_2();
   }
   while (power_of_2 > zero) {
      if (divisor <= remainder) {
         remainder = remainder - divisor;
         quotient = quotient + power_of_2;
      }
      divisor.divide_by_2();
      power_of_2.divide_by_2();
   }
   DEBUGF ('/', "quotient = " << quotient);
   DEBUGF ('/', "remainder = " << remainder);
   return {.quotient = quotient, .remainder = remainder};
}

ubigint ubigint::operator/ (const ubigint& that) const {
   return udivide (*this, that).quotient;
}

ubigint ubigint::operator% (const ubigint& that) const {
   return udivide (*this, that).remainder;
}

bool ubigint::operator== (const ubigint& that) const {
   return uvalue == that.uvalue;
}

bool ubigint::operator< (const ubigint& that) const {
   if(uvalue.size() < that.uvalue.size()){
      return true;
   } else if(uvalue.size() > that.uvalue.size()) {
      return false;
   } else {
      for(int i=static_cast<int>(uvalue.size())-1; i >= 0; i--){
         if(uvalue[i] < that.uvalue[i]){
            return true;
         } else if(uvalue[i] > that.uvalue[i]){
            return false;
         }
      }
      return false;
   }
}

void ubigint::print() const {
   DEBUGF ('p', this << " -> " << *this);
   int count = 0;
   for(auto i=uvalue.rbegin(); i!=uvalue.rend(); i++){
      count += 1;
      cout << static_cast<int> (*i);
      if(count == 69){
         cout << "\\" << endl;
         count = 0;
      }
   }
}

ostream& operator<< (ostream& out, const ubigint& that) {    
   out << "ubigint(";
   for(auto i=that.uvalue.rbegin(); i!=that.uvalue.rend(); i++){
      out << static_cast<int>(*i);
   }
   return out << ")";
}

/**
 * Jack Robbins
 * CS-280 Section 011
 * 11/15/2023
*/

#include <iostream>
#include <string>
#include "val.h"

using namespace std;

//For this operator, divide op from "this"
Value Value::operator/(const Value& op) const{
    //When we saw GetType(), think of it as this->GetType()
    //If the types match continue
    if(GetType() == op.GetType()){
        //If we have two ints, result will be an int
        if(IsInt()){
            return Value(GetInt() / op.GetInt());
        }
        //Same thing for if they're both reals
        if(IsReal()){
            return Value(GetReal() / op.GetReal());
        }
    }
    //Otherwise we could have a mismatch and still work
    if (IsReal() && op.IsInt()){
        //Cast op to be broader
        return Value(GetReal() / (float)op.GetInt());
    }

    if (IsInt() && op.IsReal()){
        //in this case cast the current object to be broader
        return Value((float)GetInt() / op.GetReal());
    }

    //If we get here, we have some string or booleans, which are invalid
    return Value();
}

//Comparing "this" with op
Value Value::operator==(const Value& op) const {
    //Check the cases for when types are equal first
    if (GetType() == op.GetType()){
        if(IsInt()){
            return Value(GetInt() == op.GetInt());
        }    

        if(IsReal()) {
            return Value(GetReal() == op.GetReal());
        }

        if(IsString()){
            return Value(GetString() == op.GetString());
        }

        if(IsBool()){
            return Value(GetBool() == op.GetBool());
        }
    }

    //Just like the /operator, we can use casting to compare reals and ints
    if (IsReal() && op.IsInt()){
        //Cast op to be broader
        return Value(GetReal() == (float)op.GetInt());
    }

    if (IsInt() && op.IsReal()){
        //in this case cast the current object to be broader
        return Value((float)GetInt() == op.GetReal());
    }

    //If we get here, something went wrong so just return an empty value
    return Value();
}

//This ANDing operator is only valid for two booleans
Value Value::operator&&(const Value& oper) const{
    //If both types are not booleans, this will not work
    if(GetType() != VBOOL || oper.GetType() != VBOOL){
        return Value();
    }

    //If they are both booleans, simply use the boolean && operator
    return Value(GetBool() && oper.GetBool());

}

//Mod can only be performed on two ints, nothing else
Value Value::operator%(const Value& oper) const{
    //If both types are not int, this will not work
    if(GetType() != VINT || oper.GetType() != VINT){
        return Value();
    }
    //If we get here, we have two ints and this is valid
    return(GetInt() % oper.GetInt());
}

/**
 * Jack Robbins
 * This contains all of the member function definition for the overloaded operators
 * that our interpreter will use
*/

#include <iostream>
#include <string>
#include "val.h"

using namespace std;


//addition may only occur between reals, ints, or a mix of the two
Value Value::operator+(const Value& op) const{
    switch(GetType()){
        //If we have an int, we can add with either a real or an int
        case VINT:
            if(op.GetType() == VINT){
                //we can have two ints, no casting needed
                return Value(GetInt() + op.GetInt());
            }

            if (op.GetType() == VREAL){
                //here we'll need to cast to the broader type
                return Value((float)GetInt() + op.GetReal());
            }
            
            //If we get here, op was either a string or a float and therefore invalid
            return Value();

        //If we have an int, we can add with either a real or an int
        case VREAL:
            if(op.GetType() == VINT){
                //cast to a float
                return Value(GetReal() + (float)op.GetInt());
            }

            if (op.GetType() == VREAL){
                //no need for casting here
                return Value(GetReal() + op.GetReal());
            }
            
            //If we get here, op was either a string or a float and therefore invalid
            return Value();

        default:
        //If we get here, this object was either a string or boolean and therefore invalid
            return Value();
    }
}


//Subtraction may occur only between ints and reals
Value Value::operator-(const Value& op) const{
     switch(GetType()){
        //If we have an int, we can add with either a real or an int
        case VINT:
            if(op.GetType() == VINT){
                //we can have two ints, no casting needed
                return Value(GetInt() - op.GetInt());
            }

            if (op.GetType() == VREAL){
                //here we'll need to cast to the broader type
                return Value((float)GetInt() - op.GetReal());
            }
            
            //If we get here, op was either a string or a float and therefore invalid
            return Value();

        //If we have an int, we can add with either a real or an int
        case VREAL:
            if(op.GetType() == VINT){
                //cast to a float
                return Value(GetReal() - (float)op.GetInt());
            }

            if (op.GetType() == VREAL){
                //no need for casting here
                return Value(GetReal() - op.GetReal());
            }
            
            //If we get here, op was either a string or a float and therefore invalid
            return Value();

        default:
        //If we get here, this object was either a string or boolean and therefore invalid
            return Value();
    }
}





//division may occur only between ints and reals
Value Value::operator/(const Value& op) const{
switch(GetType()){
        //If we have an int, we can add with either a real or an int
        case VINT:
            if(op.GetType() == VINT){
                //we can have two ints, no casting needed
                return Value(GetInt() / op.GetInt());
            }

            if (op.GetType() == VREAL){
                //here we'll need to cast to the broader type
                return Value((float)GetInt() / op.GetReal());
            }
            
            //If we get here, op was either a string or a float and therefore invalid
            return Value();

        //If we have an int, we can add with either a real or an int
        case VREAL:
            if(op.GetType() == VINT){
                //cast to a float
                return Value(GetReal() / (float)op.GetInt());
            }

            if (op.GetType() == VREAL){
                //no need for casting here
                return Value(GetReal() / op.GetReal());
            }
            
            //If we get here, op was either a string or a float and therefore invalid
            return Value();

        default:
        //If we get here, this object was either a string or boolean and therefore invalid
            return Value();
    }
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

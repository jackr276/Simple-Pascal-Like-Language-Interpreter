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
            
        //If we have a real, we can add with either a real or an int
        case VREAL:
            if(op.GetType() == VINT){
                //cast to a float
                return Value(GetReal() + (float)op.GetInt());
            }

            if (op.GetType() == VREAL){
                //no need for casting here
                return Value(GetReal() + op.GetReal());
            }

        default:
        //If we get here, this object was either a string or boolean and therefore invalid
            return Value();
    }

    //If we get here, op was either a string or a float and therefore invalid
    return Value();
}


//Subtraction may occur only between ints and reals
Value Value::operator-(const Value& op) const{
     switch(GetType()){
        //If we have an int, we can subtract with either a real or an int
        case VINT:
            if(op.GetType() == VINT){
                //we can have two ints, no casting needed
                return Value(GetInt() - op.GetInt());
            }

            if (op.GetType() == VREAL){
                //here we'll need to cast to the broader type
                return Value((float)GetInt() - op.GetReal());
            }
            
        //If we have a real, we can subtract with either a real or an int
        case VREAL:
            if(op.GetType() == VINT){
                //cast to a float
                return Value(GetReal() - (float)op.GetInt());
            }

            if (op.GetType() == VREAL){
                //no need for casting here
                return Value(GetReal() - op.GetReal());
            }

        default:
        //If we get here, this object was either a string or boolean and therefore invalid
            return Value();
    }
                
    //If we get here, op was either a string or a float and therefore invalid
    return Value();
}


//Multiplication may only occur between ints and reals
Value Value::operator*(const Value& op) const{
    switch(GetType()){
        //If we have an int, we can multiply with either a real or an int
        case VINT:
            if(op.GetType() == VINT){
                //we can have two ints, no casting needed
                return Value(GetInt() * op.GetInt());
            }

            if (op.GetType() == VREAL){
                //here we'll need to cast to the broader type
                return Value((float)GetInt() * op.GetReal());
            }
            
        //If we have a real, we can multiply with either a real or an int
        case VREAL:
            if(op.GetType() == VINT){
                //cast to a float
                return Value(GetReal() * (float)op.GetInt());
            }

            if (op.GetType() == VREAL){
                //no need for casting here
                return Value(GetReal() * op.GetReal());
            }

        default:
        //If we get here, this object was either a string or boolean and therefore invalid
            return Value();
    }
    
    //If we get here, op was either a string or a boolean and therefore invalid
    return Value();
}


//division may occur only between ints and reals
Value Value::operator/(const Value& op) const{
    switch(GetType()){
        //If we have an int, we can divide with either a real or an int
        case VINT:
            if(op.GetType() == VINT){
                //we can have two ints, no casting needed
                return Value(GetInt() / op.GetInt());
            }

            if (op.GetType() == VREAL){
                //here we'll need to cast to the broader type
                return Value((float)GetInt() / op.GetReal());
            }

        //If we have a real, we can divide with either a real or an int
        case VREAL:
            if(op.GetType() == VINT){
                //cast to a float
                return Value(GetReal() / (float)op.GetInt());
            }

            if (op.GetType() == VREAL){
                //no need for casting here
                return Value(GetReal() / op.GetReal());
            }

        default:
        //If we get here, this object was either a string or boolean and therefore invalid
            return Value();
    }
        
    //If we get here, op was either a string or a boolean and therefore invalid
    return Value();
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


//Performs numeric integer division on this by the operator
// FIXME potentially here -> not entirely sure how this is supposed to work or how its different from idiv 
Value Value::div(const Value& oper) const{
    //We can only work with ints and reals, but everything must be cast to an int in the end
    switch(GetType()){
        case VINT:
            if(oper.GetType() == VINT){
                //we can have two ints, no casting needed
                return Value(GetInt() / oper.GetInt());
            }

            if (oper.GetType() == VREAL){
                //here we'll need to cast to an int
                return Value(GetInt() / (int)oper.GetReal());
            }
        
        case VREAL:
            if(oper.GetType() == VINT){
                //cast to an int
                return Value((int)GetReal() / oper.GetInt());
            }

            if (oper.GetType() == VREAL){
                //both need to be int casted
                return Value((int)GetReal() / (int)oper.GetReal());
            }

        default:
        //If we get here, this object was either a string or boolean and therefore invalid
            return Value();
    }
                
    //If we get here, op was either a string or a boolean and therefore invalid
    return Value();
}


//Comparing "this" with op
Value Value::operator==(const Value& op) const {
    //Check the cases for when types are equal first
    if (GetType() == op.GetType()){
        switch(GetType()){
            case VINT:
                return Value(GetInt() == op.GetInt());
            
            case VREAL:
                return Value(GetReal() == op.GetReal());
            
            case VSTRING:
                return Value(GetString() == op.GetString());

            case VBOOL:
                return Value(GetBool() == op.GetBool());

            default:
                //we should never get here in theory, added to avoid compile warnings on Vocareum
                return Value();
        }
    }
    
    //There are two special cases were we can mix ints and reals in either order
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


//Comparing "this" with op, numeric types only for >
Value Value::operator>(const Value& op) const{
    //We can only compare with ints and reals
    switch (GetType()){
        case VINT:
            if(op.GetType() == VINT){
                return Value(GetInt() > op.GetInt());
            }

            if (op.GetType() == VREAL){
                return Value((float)GetInt() > op.GetReal());
            }

        case VREAL:
            if(op.GetType() == VINT){
                return Value(GetReal() > (float)op.GetInt());
            }

            if(op.GetType() == VREAL){
                return Value(GetReal() > op.GetReal());
            }
                
        default:
            //if we end up here, we have either a boolean or a string
            return Value();
    }

    //If we got here, we had either a boolean or a string
    return Value();
}


//Comparing "this" with op, numeric types only for <
Value Value::operator<(const Value& op) const{
    //We can only compare with ints and reals
    switch (GetType()){
        case VINT:
            if(op.GetType() == VINT){
                return Value(GetInt() < op.GetInt());
            }

            if (op.GetType() == VREAL){
                return Value((float)GetInt() < op.GetReal());
            }

        case VREAL:
            if(op.GetType() == VINT){
                return Value(GetReal() < (float)op.GetInt());
            }

            if(op.GetType() == VREAL){
                return Value(GetReal() < op.GetReal());
            }
        
        default:
            //if we end up here, we have some noncompatible types
            return Value();
    }

    //If we got here, we had either a boolean or a string
    return Value();
}


//Performs numeric integer division on this by the operator
// FIXME potentially here -> not entirely sure how this is supposed to work or how its different from idiv 
Value Value::idiv(const Value& op) const{
    //We can only work with ints and reals, but everything must be cast to an int in the end
    switch(GetType()){
        case VINT:
            if(op.GetType() == VINT){
                //we can have two ints, no casting needed
                return Value(GetInt() / op.GetInt());
            }

            if (op.GetType() == VREAL){
                //here we'll need to cast to an int
                return Value(GetInt() / (int)op.GetReal());
            }
        
        case VREAL:
            if(op.GetType() == VINT){
                //cast to an int
                return Value((int)GetReal() / op.GetInt());
            }

            if (op.GetType() == VREAL){
                //both need to be int casted
                return Value((int)GetReal() / (int)op.GetReal());
            }

        default:
        //If we get here, this object was either a string or boolean and therefore invalid
            return Value();
    }
        
    //If we get here, op was either a string or a float and therefore invalid
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


//This ORing operator is only valid for two booleans
Value Value::operator||(const Value& oper) const{
    //If both types are not booleans, this will not work
    if(GetType() != VBOOL || oper.GetType() != VBOOL){
        return Value();
    }

    //If they are both booleans, simply use the boolean || operator
    return Value(GetBool() || oper.GetBool());
}


//This copmlement operator is only valid for a boolean
Value Value::operator!() const{
    if(GetType() == VBOOL){
        //Return the flipped boolean
        return Value(!GetBool());
    }

    //If we end up here, we didn't have a boolean and therefore have an error
    return Value();
}

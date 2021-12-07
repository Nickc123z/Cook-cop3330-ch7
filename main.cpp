/*
 *  UCF COP3330 Fall 2021 Assignment 6 Solution
 *  Copyright 2021 Nick Cook
 */ 



#include "std_lib_facilities.h"

// constants for excercise
const char number = '8';    
const char quit   = 'q';    
const char print  = ';';    
const char name   = 'a';    
const char let    = 'L';   
const char con    = 'C';  
const string declkey = "let";	
const string constkey = "const";	
const string prompt  = "> ";
const string result  = "= ";

// token class
class Token {
public:
    char kind;       
    double value;     
    string name;     
    Token(char ch)             : kind(ch), value(0)   {}
    Token(char ch, double val) : kind(ch), value(val) {}
    Token(char ch, string n)   : kind(ch), name(n)    {}
};

// token class
class Token_stream {
public: 
    Token_stream();   
    Token get();      
    void putback(Token t);    
    void ignore(char c);    
private:
    bool full;        
    Token buffer;    
};

// token stream call
Token_stream::Token_stream()
:full(false), buffer(0)    
{
}

// token stream
void Token_stream::putback(Token t)
{   
    // catch error
    if (full) error("putback() into a full buffer");
    buffer = t;      
    full = true;     
}

Token Token_stream::get() 
{   
  //catch error
    if (full) {        
        full=false;
        return buffer;
    }  

    // gets input
    char ch;
    cin >> ch;  
    // catches CH char      
    switch (ch) {
    case quit:
    case print:
    case '(':
    case ')':
    case '+':
    case '-':
    case '*':
    case '/': 
    case '%':
    case '=':
        return Token(ch); 
    case '.':            
    case '0': 
    case '1': 
    case '2': 
    case '3': 
    case '4':
    case '5': 
    case '6': 
    case '7': 
    case '8': 
    case '9':   
    {
        cin.putback(ch);
        double val;
        cin >> val;    
        return Token(number,val);
    }
    default:
        if (isalpha(ch)) {	
            string s;
            s += ch;
            while (cin.get(ch) && (isalpha(ch) || isdigit(ch) || ch=='_')) s+=ch;
            cin.putback(ch);
            if (s == declkey) return Token(let); 
            if (s == constkey) return Token(con); 
            return Token(name,s);
        }
        error("Bad token");
    }
}

// Token_stream function
void Token_stream::ignore(char c)
   
{   
  //catches error
    if (full && c==buffer.kind) {
        full = false;
        return;
    }
    full = false;
    char ch = 0;
    while (cin>>ch)
        if (ch==c) return;
}

Token_stream ts;     

//class variable
class Variable {
public:
    string name;
    double value;
	bool var;	
    Variable (string n, double v, bool va = true) :name(n), value(v), var(va) { }
};

// vector variable
vector<Variable> var_table;

//dbl get_value function
double get_value(string s)

{
    //returns variable if X
    for (int i = 0; i<var_table.size(); ++i)
        if (var_table[i].name == s) return var_table[i].value;
    // catches error
    error("get: undefined variable ", s);
}


// set_value function
void set_value(string s, double d)
 
{
    for (int i = 0; i<var_table.size(); ++i)
        if (var_table[i].name == s) {
      //catches error
			if (var_table[i].var==false) error(s," is a constant");
            var_table[i].value = d;
            return;
        }
    //catch error
    error("set: undefined variable ", s);
}
// bool function
bool is_declared(string var)
{   
  // assigning name to var
    for (int i = 0; i<var_table.size(); ++i)
        if (var_table[i].name == var) return true;
    return false;
}

//dbl define_name function
double define_name(string s, double val, bool var=true)
{
    // catches error
    if (is_declared(s)) error(s," declared twice");
    var_table.push_back(Variable(s,val,var));
    return val;
}
//dbl expression function
double expression();    
//dbl primary function
double primary()
{
    Token t = ts.get();
    // switch statement
    switch (t.kind) {
    // case (
    case '(':          
        {
            double d = expression();
            t = ts.get();
            //catch error
            if (t.kind != ')') error("')' expected");
            return d;
        }
    // case if number
    case number:    
        return t.value;  
    //case if name
    case name:
		{
			Token next = ts.get();
      // catches if input is =
			if (next.kind == '=') {	
				double d = expression();
				set_value(t.name,d);
				return d;
			}
			else {
				ts.putback(next);	
				return get_value(t.name);
			}
		}
    //case -
    case '-':
        return - primary();
    //case +
    case '+':
        return primary();
    //catch error. print error
    default:
        error("primary expected");
    }
}

// dbl function term
double term()
{
    double left = primary();
    Token t = ts.get(); 

    //always true!
    while(true) {
        switch (t.kind) {
        //case *
        case '*':
            left *= primary();
            t = ts.get();
            break;
        //case /
        case '/':
            {    
                double d = primary();
                //catch error, print error
                if (d == 0) error("divide by zero");
                left /= d; 
                t = ts.get();
                break;
            }
        // case %
        case '%':
            {    
                int i1 = narrow_cast<int>(left);
                int i2 = narrow_cast<int>(term());
                // if X divides by 0, prints error
                if (i2 == 0) error("%: divide by zero");
                left = i1%i2; 
                t = ts.get();
                break;
            }
        default: 
            ts.putback(t);       
            return left;
        }
    }
}

// dbl expression function
double expression()
{
    //varaibles
    double left = term();      
    Token t = ts.get();        
    
    // always true
    while(true) {    
        // switch statement to determine which + -
        switch(t.kind) {
        // + case
        case '+':
            left += term();   
            t = ts.get();
            break;
        // - case
        case '-':
            left -= term();   
            t = ts.get();
            break;
        default: 
            ts.putback(t);    
            return left;       
        }
    }
}

//dbl declaration function
double declaration(Token k)
   
{
    Token t = ts.get();
    // catches error
    if (t.kind != name) error ("name expected in declaration");
    //variable
    string var_name = t.name;

    Token t2 = ts.get();
    //catches error
    if (t2.kind != '=') error("= missing in declaration of ", var_name);

    double d = expression();
    define_name(var_name,d,k.kind==let);
    return d;
}

//dbl statement function
double statement()
{
    Token t = ts.get();
    //switch statement to find which is used
    switch (t.kind) {
    case let:
    // returns declaration
	  case con:
        return declaration(t.kind);
    //returns expression
    default:
        ts.putback(t);
        return expression();
    }
}


//cleans up mess
void clean_up_mess()
{ 
    ts.ignore(print);
}


// calc function
void calculate()
{
    while (cin)
      try {
        cout << prompt;
        Token t = ts.get();
        while (t.kind == print) t=ts.get();
        // if quit, return    
        if (t.kind == quit) return;       
        ts.putback(t);
        cout << result << statement() << endl;
    }
    //catches error
    catch (exception& e) {
        cerr << e.what() << endl;        
        clean_up_mess();
    }
}


int main()
try {
    //variables
    define_name("pi",3.1415926535,false);	
    //calc function
    calculate();

    keep_window_open();    
    return 0;
}
//catches error
catch (exception& e) {
    cerr << e.what() << endl;
    keep_window_open("~~");
    return 1;
}
//catches error
catch (...) {
    cerr << "exception \n";
    keep_window_open("~~");
    return 2;
}

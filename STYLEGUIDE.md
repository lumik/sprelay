# Coding style guide

This style guide is inspired by the [Google style guide][gStyleGuide].

## C++ feature conventions
* Use **forward declarations** when it is possible
* place code in **namespace** based on project name
* don't use **using directivers**, don't use **name aliasses** in header file
* **unnamed namespaces** – static variables, methods which are used only inside
  source file should be included in unnamed namespace or declared static and
  shouldn't be in `.h` file.
* **local variables**:
  * place them in the narrowest scope possible (nearest scope to their usage)
    and initialize them in the declaration
  * exception – loops, where variable is declared in each iteration. Here, place
    the local variable outside the loop
* **static variables** should be only POD due to undetermined calling order of
  destructors.
* **default arguments** are banned on **virtual functions**
* **exceptions** – use them for constructors to preserve RAII but be aware of
  Qt's specifics – if exception occur in slot, it has to be handled there,
  unwinding Qt's stack is not exception safe, so if the exception propagates
  through Qt, you can then only do clean up and exit application.
* use **RTTI** as rarely, as possible, use for example the visitor pattern
  instead, with Qt, you can use Qt's equivalent for QObjects
* use C++ **type casts** or brace initialization for casts
* use prefix **++** and **--**, not postfix
* use **const** and **constexpr** whenever it's possible but don't overuse
  constexpr (for example in functions, which may be in future downgraded to not
  constexpr)
* **integer types** – from C++ integer types use only `int`, other use from
  `<cstdint>` or `Qt`

## Naming conventions:
* **names** should be descriptive, avoid abbreviations, use abbreviations only
  generally known and not ambiguous
* **file** names – all lowercase, can include underscores (`_`) and dashes
  (`-`), prefer underscores
* **type** names – CamelCase with no underscores, i.e. type names should start
  with a capital letter and have a capital letter for each new word. It applies
	to names of all custom types as classes, structs, type aliases, enums, and
	type template	parameters.
* **variable** names – all lowercase with underscore between words, a class
  private data members (not methods) should have trailing underscore, struct
  data members are withou trailing underscore
* **constant** names – leading k + CamelCase – this applies especially for
  static storage duration
* **function** names:
  * class methods - mixedCase (CamelCase with leading lowercase) it also
    applies for acronyms
  * other functions - all lower case as variables.
* **getters** and **setters** – as variables
* **namespace** names – all lowercase with underscores between words
* **macro** names – capitals with underscores between names MY_MACRO
* **enumerator** names – like macros
* **include guards** – header include guards should be named after the filename
  ending with _H_ suffix and prefixed with subdirectory names. For exmaple if
  you have file `my_app/my_class.h`, you should use `MY_APP_MY_CLASS_H_`
    
## Formating conventions
* **comments**
  * start each file with license boilerplate
  * at the beginning describe the file content
  * inline comments – put 2 spaces between code and comment
  ```
  int i;  // inline comment
  // line comment
  ```
* prevent multiple inclusion of header by C++ **include guards**, terminate the
  `#endif` with comment with the macro name
  ```cpp
  #ifndef MY_HEADER_H_
  #define MY_HEADER_H_
  // some code
  #endif  // MY_HEADER_H_
  ```
* terminate **namespace** with comment with the namespace name
  ```cpp
  namespace my_namespace {
  
  int my_function();
  
  
  class MyClass
  {
      MyClass();
  };
  }  // namespace my_namespace
  ```
* **Order of header includes**:
  1. header belonging to the source file
  1. C includeds
  2. C++ includes
  3. other libraries
  4. project headers
* **Order of acces control specifiers**:
  1. public
  2. protected
  3. private
* **Order of declarations**:
  1. types (typedef, using, nested structs and classes, ...)
  2. constants
  3. factory functions (do job from constructor, can be virtual and overriden
     from child classes)
  4. constructors
  5. assignment operators
  6. destructor
  7. all other methods
  8. data members
* **parameters ordering**:
  1. inputs
  2. outputs
* if the function is longer then 40 lines, think of breaking it
* **lines** should be maximally **120 characters long**
* use 4 spaces for **indentation**, never tabs
* **function declarations** and **headers** – function signature parentheses
  are closely attached to the function name (without space):
  ```cpp
  void f(int var1, int var2);  // use variable names also in prototypes
  void g(int var1, int var2,
         int var3);            // if you break line inside parameter list,
                               // indet new line to the level of the most
                               // inside enclosing parentheses
  void h(
      int var1, int var2, int var3);  // or break the line right after openning
                                      // parenthesis and indent by 4 spaces
  ```
* **function definition** – block opening parenthesis is on the new line and
  closing parenthesis is on its own line and is not indented. Function body
  is indented.
  ```cpp
  void f()
  {
      // implementation ...
  }
  ```
* the same rules for **line breaking** as for function parameters are valid
  also for other similar constructs like expressions in parentheses, control
  structures, return statements, assignment operators, ...
* **inline function definitions** (this apply also for inline functions in
  classes)
  * if definition is also declaration
  ```cpp
  inline int f() { return 5; }  // if it can be at one line, let it be at one
                                // line, make white spaces around curly
                                // brackets
  inline int f()
  {
      return 5;                 // if it can't be at one line, make it the same
  }                             // as for normal functions
  ```
  * if definition and declaration is distinct, use the same rules as for
    functions
* **control structures**
  * indent parentheses from control structure keyword by one space
  ```cpp
  for (int i = 0; i < n; ++i) do_something();  // one line
  for (int i = 0; i < n; ++i)
      do_something();                          // two lines  
  ```
  * place block openning parentheses at the same line as control structure
    divided by one space from the structure opening
  ```cpp
  for (int i = 0; i < n; ++i) {
      do_something();                          // block
  }
  ```
  * switch – use space before colon
  ```cpp
  switch (var) {
      case 0 :
      case 1 :
          do_something();
          break;
      default :
          assert(false);  // may never occur, use assert
  }
  ```
* **break before binary operations** – it is easier to match operators with
  operands [\[Knuth, 1996, p. 196 and 197\]](#Knuth%3A1996)
  ```cpp
  income = gross_wages
           + taxable_interest
           + (dividends - qualified_dividends)
           - ira_deduction
           - student_loan_interest;
  ```
* **pointers** and **references** – declare or use as function parameters with
  dereference operator or address operator attached to the identifier except
  for pointers to array or functions returning pointer, where these operators
  have less precedence then the other operator attached to the identifier. Use
  spaces between type and `*` or `&`.
  ```cpp
  int *i;
  int * i[5];
  const int & f(const int &i);
  ```
* **variable initialization** – attach directly to variable using parentheses
  initialization `()` or list initialization `{}`
  ```cpp
  int i(5);
  int j{5};
  ```
* **class** formating:
  * openning curly bracket should be at its own line
  * public, protected and private specifiers without indentation
  * they should be preced by blank line (except for the first one)
  * initializer list:
  ```cpp
  class MyClass
  {
      MyClass(int var1) : var1_(var1), var2_(20) {}  // empty definition at the
                                                     // same line
      MyClass(int var1, int var2)
          : var1_(var2), var2_(var2),  // If line break is needed, place the
            var3_(30)                  // colon at new line
      {}                               // and empty definition at another new
                                       // line too
  };  // class MyClass
  ```
  * inheritance – the same rules as for member initializer list
  * end class declaration with comment with the class name
* **namespaces** doesn't add new indent
* use 2 **new lines** between definitions in `.cpp` file
* **range based for loop** has spaces before and after colon
  ```cpp
  for (auto x : vec) { do_something(x); }
  ```
* **operators** – use spaces around binary operators, parentheses should have
  no spaces, no spaces separating unary operators and their arguments
  ```cpp
  v = w * x + y / z;
  v = w * (x + z);
  x = -5;
  ++x;
  if (x && !y) do_something();
  ```
* **templates** and **type casts** – use no spaces between `<>()`
  ```cpp
  std::vector<string> x;
  y = static_cast<char *>(x);
  ```
* termine the file with new line

## References

<a name="Knuth:1996"></a>Donald Knuth's The TeXBook, 1996.


[gStyleGuide]: https://google.github.io/styleguide/cppguide.html

# Coding style guide

This style guide is inspired by the [Google style guide][g_style_guide].


## C++ feature conventions

* use **forward declarations** when it is possible.
* place code in **namespace** based on project name.
* don't use **using directivers**, don't use **name aliasses** in header file.
* **local variables**:
  * place them in the narrowest scope possible (nearest scope to their usage)
    and initialize them in the declaration.
  * exception – loops, where variable is declared in each iteration. Here, place
    the local variable outside the loop.
* **static variables** should be only POD due to undetermined calling order of
  destructors.
* **default arguments** are banned on **virtual functions**.
* **exceptions** – use them for constructors to preserve RAII but be aware of
  Qt's specifics – if exception occur in slot, it has to be handled there,
  unwinding Qt's stack is not exception safe, so if the exception propagates
  through Qt, you can then only do clean up and exit application.
* use **RTTI** as rarely, as possible, use for example the visitor pattern
  instead, with Qt, you can use Qt's equivalent for QObjects.
* use C++ **type casts** or brace initialization for casts.
* use prefix **`++`** and **`--`**, not postfix.
* use **`const`** and **`constexpr`** whenever it's possible but don't overuse
  constexpr (for example in functions, which may be in future downgraded to not
  constexpr).
* **integer types** – from C++ integer types use only `int`, other use from
  `<cstdint>` or `Qt`.
* do not use unsigned types (e. g. `std::size_t`) for indices, there is a risk
  of unintended modulo arithmetics, see
	[C++ Core Guidelines][core_guidelines-size_t].
* **`assert`** liberally – use `assert` macro to its fullest. Check all your
  conditions and assumptions
  ```cpp
  inline Value *getOperand(unsigned i) {
      assert(i < operands_.size() && "getOperand() out of range!");
      return operands_[i];
  }
  ```
* provide a virtual method anchor for classes in header – If a class is defined
  in a header file and has vtable (either it has virtual methods or it derives
	from classes with virtual methods), it must always have at least one
	out-of-line (not inline) virtual method in the class to make hint to compiler
	where to place the vtable. Without this, the compiler will copy the vtable
	and RTTI into every `.o` file that includes the header, bloating `.o` file
	sizes and increasing link times.
* don’t use **`default`** labels in fully covered switches over enumerations –
  The-Wswitch flag warns if a switch, without a default label, over an
	enumeration does not cover every enumeration value and it won't fire if you
	use default and add new value to enumeration which you forget to cover.
* don’t evaluate `end()` every time through a loop – In cases where range-based
  for loops can’t be used and it is necessary to write an explicit
	iterator-based loop, pay close attention to whether `end()` is re-evaluted
	on each loop iteration. One common mistake is to write a loop in this style:
	```cpp
	BasicBlock *bb = // ...
  for (auto i = bb->begin(); i != bb->end(); ++i)
      // ... use i ...
	```
	The problem with this construct is that it evaluates `bb->end()` every time
	through the loop. Instead of writing the loop like this, we strongly prefer
	loops to be written so that they evaluate it once before the loop starts. A
	convenient way to do this is like so:
	```cpp
	BasicBlock *bb = ...
      for (auto i = bb->begin(), e = bb->end(); i != e; ++i)
          // ... use i ...
	```
	The observant may quickly point out that these two loops may have different
	semantics: if the container (a basic block in this case) is being mutated,
	then `bb->end()`` may change its value every time through the loop and the
	second loop may not in fact be correct. If you actually do depend on this
	behavior, please write the loop in the first form and add a comment
	indicating that you did it intentionally.

  Why do we prefer the second form (when correct)? Writing the loop in the
	first form has two problems. First it may be less efficient than evaluating
	it at the start of the loop. In this case, the cost is probably minor — a few
	extra loads every time through the loop. However, if the base expression is
	more complex, then the cost can rise quickly. I’ve seen loops where the end
	expression was actually something like: `some_map[x]->end()` and map lookups
	really aren’t cheap. By writing it in the second form consistently, you
	eliminate the issue entirely and don’t even have to think about it.

  The second (even bigger) issue is that writing the loop in the first form
	hints to the reader that the loop is mutating the container (a fact that a
	comment would handily confirm!). If you write the loop in the second form, it
	is immediately obvious without even looking at the body of the loop that the
	container isn’t being modified, which makes it easier to read the code and
	understand what it does.
* avoid **`std::endl`** – The `std::endl` modifier, when used with iostreams
  outputs a newline to the output stream specified. In addition to doing this,
	however, it also flushes the output stream. Use `'\n'` literal instead. It
	can be used only when you realy want to flush output stream.
* **anonymous namespaces** – Use `static` when possible instead of anonymous
  namespace because it is localy more readable. While `static` is available in
	C++, anonymous namespaces are more general: they can make entire classes
	private to a file. Make anonymous namespaces as small as possible, and only
	use them for class declarations.


## Naming conventions:

* **names** should be descriptive, avoid abbreviations, use abbreviations only
  generally known and not ambiguous.
* **file** names – all lowercase, can include underscores (`_`) and dashes
  (`-`), prefer underscores.
* **type** names – CamelCase with no underscores, i.e. type names should start
  with a capital letter and have a capital letter for each new word. It applies
	to names of all custom types as classes, structs, type aliases, enums, and
	type template	parameters.
* **variable** names – all lowercase with underscore between words, a class
  private data members (not methods) should have trailing underscore, struct
  data members are withou trailing underscore.
* **constant** names – leading k + CamelCase – this applies especially for
  static storage duration.
* **function** names:
  * class methods - mixedCase (CamelCase with leading lowercase) it also
    applies for acronyms.
  * other functions - all lower case as variables.
* **getters** and **setters** – as variables.
* **namespace** names – all lowercase with underscores between words.
* **macro** names – capitals with underscores between names MY_MACRO, other
  names shouldn't be in all capitals to distiguish that only macros are not
  subjected to namespaces.
* **enumerator** names – CamelCase.
* **include guards** – header include guards should be named after the filename
  ending with _H_ suffix and prefixed with subdirectory names. For exmaple if
  you have file `my_app/my_class.h`, you should use `MY_APP_MY_CLASS_H_`.
* **template parameter** names – CamelCase with leading `T` for type or
  template arguments and with leading `t` for non-type arguments.

    
## Formating conventions

* **comments**
  * start each file with license boilerplate.
  * at the beginning describe the file content.
  * inline comments – put 2 spaces between code and comment
  ```cpp
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
  Place include guards only inside headers (`.h` extension) not heleper files
  which are included in the middle of the headers (for example `.tpp`).
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
  Each category should be sorted lexicographically by the full path. Other
  libraries can be divided into more subcategories.
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
* **lines** should be maximally **120 characters long**.
* use 4 spaces for **indentation**, never tabs.
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
  classes).
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
* **templates** declaration – do not use space between template and `<`, place
  identifier to a new line. Use `typename` keyword instead of `class` keyword.
  ```cpp
  template <typename TType, int tScalarArgument>
	class MyClassTemplate;
  ```
* termine the file with new line.
* return true for success or zero for error codes from functions (if you can't
  use exceptions). Error codes should be replaced by enum if exceptions can't
  be used.
* use early exits and `continue` to simplify code
  ```cpp
	Value * do_something(Instruction *i) {
      // We conservatively avoid transforming instructions with multiple uses
      // because goats like cheese.
      if (!i->hasOneUse())
          return 0;

      // This is really just here for example.
      if (!do_other_thing(i))
          return 0;

      // ... some long code ....
	}
	```
	or
	```cpp
	for (Instruction &i : bb) {
      auto *bo = dynamic_cast<BinaryOperator>(&i);
      if (!bo) continue;

      Value *lhs = bo->getOperand(0);
      Value *rhs = bo->getOperand(1);
      if (lhs == rhs) continue;

      // ...
  }
	```
* don't use `else` after `return` – see example above about early exits
* turn predicate loops into predicate functions – it forces you to name the
  predicate and make its function clear, you can make them static to restrict
	them to file scope. This rule improves reusability of the test. Convert this:
  ```cpp
  bool found_foo = false;
  for (unsigned i = 0, e = bar_list.size(); i != e; ++i) {
      if (bar_list[i]->isFoo()) {
          found_foo = true;
          break;
      }
  }

  if (found_foo) {
      // ...
  }
  ```
  to this:
  ```cpp
  /// \returns true if the specified list has an element that is a foo.
  static bool contains_foo(const std::vector<Bar*> &list) {
      for (unsigned i = 0, e = List.size(); i != e; ++i) {
          if (list[i]->isFoo()) return true;
      }
      return false;
  }
  // ...

  if (contains_foo(bar_list)) {
      // ...
  }
  ```
* don’t use `inline` when defining a function in a class definition	– A member
  function defined in a class definition is implicitly inline.


## Documentation

- write brief of global and namespace members inside header before declaration.
- you can write brief of other things inside headers.
- all other documentation should be in source files except from doxygen bug
  workarounds (scoped enumeration members with the same name as the member
	from the other scoped enumeration)
- place also `\headerfile ""` macro after `\brief` for the non-public classes
  (classes which are not in headers distributed with the library) to change
  their include directive in the documentation from <> to "".
- use doxygen only for public class member documentation. Private stuff
  document only with comments. Document only the API and implementation
  consequences for the user, do not document the implementation details using
  doxygen.
- create self documenting names and code, do not comment obvious, do not
  repeat obvious in comments. Comment overview and leading ideas of an
  implementation.
- divide documentation into modules only when necessary, do not duplicate
  namespace structure by modules. Create main module for the whole project.


## References

<a name="Knuth:1996"></a>Donald Knuth's The TeXBook, 1996.


[g_style_guide]: https://google.github.io/styleguide/cppguide.html
[core_guidelines-size_t]: https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md#arithmetic

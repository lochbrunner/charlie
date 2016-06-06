# charlie
A learning project: Compiles C++ code to byte-code and runs it in a virtual machine.


## Usage

### The Console

Type into the console

charlie build <entry>
    where `<entry>
        ` is your entry C++ file in which a `main` function must be defined.

### The Library

Usage example:
``` c++
using namespace charlie;

// Init the Compiler and set the output for compiler messages.
Compiler compiler = Compiler([](string const &message)
{
    cout << message << endl;
});

// Add external function which can be called by the program
compiler.ExternalFunctionManager.AddFunction("print", [](const char* message)
{
    cout << message;
});

// Compile the file
compiler.Build("main");

// Run the program
compiler.Run();
```

## Supported Features



## Contributing Code

Please feel welcome to folk this repository.
You will need to download the [Google Test](https://github.com/google/googletest) library in order to run the tests.
If you are using Visual Studio you have to set the system variable `GTEST` to the directory `googletest\googletest`.

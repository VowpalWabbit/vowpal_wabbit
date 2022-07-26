find_program(DOTNET_COMMAND "dotnet" REQUIRED)

# t4 will be called by dotnet. Make sure it exists so dotnet will not produce an error.
find_program(DOTNET_T4_COMMAND "t4" REQUIRED)

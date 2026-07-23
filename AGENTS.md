# Code Guidelines
- Always investigate before making changes. Add logging and test cases as needed.
- Write test cases (both end-to-end and unit) to demonstrate bugs before fixing those bugs.
- Avoid duplicating work. Put shared utilities in the main `/src` folder.
- Comment well, but don't overcomment. Assume the reader knows C++ and can read the code.
- Split code into multiple files. Avoid long files if possible.
- Always update the relevant documentation in the `/docs` folder. Create new documentation if need be.

# Commands
- Build with `cmake --build build`.
- Run the tests with `build/Tests`.
- Run a file in the project's programming language with `build/CLI -t <file>`. Add `-e` for verbose mode.
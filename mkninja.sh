find . -name "*.c" | parallel "echo build build/{/.}.o: cc {}" > sources.ninja

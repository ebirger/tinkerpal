# sed script to convert a text file to a C string

s/\\/\\\\/g        # escapes backslashes
s/"/\\"/g        # escapes quotes
s/  //g        # converts tabs to nothing
s/^/"/            # adds quotation mark to the beginning of a line
s/$/\\n"/        # adds \n and a quotation mark to the end of a line

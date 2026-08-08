#ifndef LEXER_H
# define LEXER_H

# include <vector>
# include <filesystem>
# include <istream>

std::vector<std::filesystem::path> includes(std::istream& ifs);

#endif // LEXER_H

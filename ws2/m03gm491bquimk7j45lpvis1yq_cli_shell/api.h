#ifndef M03GM491BQUIMK7J45LPVIS1YQ_CLI_SHELL_API_H
# define M03GM491BQUIMK7J45LPVIS1YQ_CLI_SHELL_API_H

# include <m03gm33dj5xo77vegpbspger4r_cli/api.h>

# include <cstddef>
# include <filesystem>
# include <iosfwd>
# include <optional>
# include <string>

namespace m03gm491bquimk7j45lpvis1yq_cli_shell {

/**
 * @brief Runs a readline shell for a cli application.
 */
class shell_t {
public:
    /**
     * @brief Constructs a shell over an application.
     */
    shell_t(m03gm33dj5xo77vegpbspger4r_cli::application_t& application, std::string prompt);

    /**
     * @brief Sets the maximum history entry count.
     */
    void history_size(std::size_t size);

    /**
     * @brief Sets the persistent history file.
     */
    void history_file(std::filesystem::path path);

    /**
     * @brief Runs the shell with standard streams.
     */
    int run();

    /**
     * @brief Runs the shell with command output streams.
     */
    int run(std::ostream& out, std::ostream& err);

private:
    m03gm33dj5xo77vegpbspger4r_cli::application_t& m_application;
    std::string m_prompt;
    std::optional<std::size_t> m_history_size;
    std::optional<std::filesystem::path> m_history_file;
};

} // namespace m03gm491bquimk7j45lpvis1yq_cli_shell

#endif // M03GM491BQUIMK7J45LPVIS1YQ_CLI_SHELL_API_H

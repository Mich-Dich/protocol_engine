
#include "util/pch.h"
#include "argument_parser.h"


// FORWARD DECLARATIONS ================================================================================================

namespace PE::argument_parser {

    // CONSTANTS =======================================================================================================

    // MACROS ==========================================================================================================

    // TYPES ===========================================================================================================

    class loc_error_category : public std::error_category {
    public:

        const char* name() const noexcept override { return "ArgParser"; }


        std::string message(int ev) const override {

            switch (static_cast<arg_error>(ev)) {
                case arg_error::success:                return "success";
                case arg_error::unknown_argument:       return "Unknown argument";
                case arg_error::missing_required:       return "Missing required argument";
                case arg_error::invalid_type:           return "Invalid type conversion";
                case arg_error::missing_value:          return "Missing value for argument";
                case arg_error::duplicate_argument:     return "Duplicate argument";
                default:                                return "Unknown error";
            }
        }
    };

    // STATIC VARIABLES ================================================================================================

    // INTERNAL FUNCTION DECLARATION ===================================================================================

    // INTERNAL FUNCTION IMPLEMENTATION ================================================================================

    const std::error_category& arg_error_category() {

        static loc_error_category instance;
        return instance;
    }


    // Helper: convert string to value based on type
    static std::error_code convert_value(const std::string& raw, const std::string& type_name, value& out) {

        if (type_name == "string")
            out = raw;

        else if (type_name == "int") {

            int val;
            auto [ptr, ec] = std::from_chars(raw.data(), raw.data() + raw.size(), val);
            if (ec != std::errc())
                return make_error_code(arg_error::invalid_type);

            out = val;

        } else if (type_name == "double") {

            double val;
            auto [ptr, ec] = std::from_chars(raw.data(), raw.data() + raw.size(), val);
            if (ec != std::errc())
                return make_error_code(arg_error::invalid_type);

            out = val;
        
        } else if (type_name == "bool") {

            if (raw == "true" || raw == "1" || raw == "yes")
                out = true;
            else if (raw == "false" || raw == "0" || raw == "no")
                out = false;
            else
                return make_error_code(arg_error::invalid_type);

        } else if (type_name == "path") {

            out = std::filesystem::path(raw);

        } else if (type_name == "uuid") {

            PE::UUID uuid = PE::util::from_string<PE::UUID>(raw);
            if (uuid == 0U)
                return make_error_code(arg_error::invalid_type);

            out = uuid;

        } else
            return make_error_code(arg_error::internal_error);

        return make_error_code(arg_error::success);
    }

    // FUNCTION IMPLEMENTATION =========================================================================================

    std::error_code make_error_code(arg_error e) {

        return std::error_code(static_cast<int>(e), arg_error_category());
    }


    parsed_result parse(const std::vector<argument_spec>& specs, int argc, char* argv[], std::error_code& error) {

        parsed_result out{};

        // Build lookup maps
        std::unordered_map<std::string, const argument_spec*> name_to_spec;
        std::unordered_map<std::string, const argument_spec*> short_to_spec;
        std::unordered_map<int, const argument_spec*> positional_specs;

        for (const auto& spec : specs) {

            name_to_spec[spec.name] = &spec;
            if (!spec.short_name.empty())
                short_to_spec[spec.short_name] = &spec;

            if (spec.positional && spec.position >= 0)
                positional_specs[spec.position] = &spec;

            if (!spec.required && spec.default_value.index() != std::variant_npos)       // Initialize with default values
                out.values[spec.name] = spec.default_value;
        }

        // Parse argv[1..]
        int pos = 0;
        for (int index = 1; index < argc; index++) {

            std::string arg = argv[index];
            const argument_spec* current_spec = nullptr;

            // Check if it's a named argument (starts with - or --)
            if (arg.size() > 1 && arg[0] == '-') {

                std::string key;
                bool longForm = (arg.size() > 2 && arg[1] == '-');
                if (longForm)
                    key = arg.substr(2);
                else
                    key = arg.substr(1); // single dash, e.g. -f

                // Find spec
                std::unordered_map<std::string, const argument_spec*>::const_iterator it;
                if (longForm) {

                    it = name_to_spec.find(key);
                    if (it == name_to_spec.end()) {

                        error = make_error_code(arg_error::unknown_argument);
                        return out;
                    }

                } else {

                    // short form: try short name first
                    it = short_to_spec.find(key);
                    if (it == short_to_spec.end()) {

                        // not found as short name, try as full name
                        it = name_to_spec.find(key);
                        if (it == name_to_spec.end()) {

                            error = make_error_code(arg_error::unknown_argument);
                            return out;
                        }
                    }
                }
                current_spec = it->second;

                // Check if this flag expects a value
                bool takes_value = (current_spec->type != "bool");
                if (takes_value) {

                    // Next token should be value
                    if (index + 1 >= argc) {

                        error = make_error_code(arg_error::missing_value);
                        return out;
                    }

                    std::string raw_value = argv[++index];
                    value val;
                    auto ec = convert_value(raw_value, current_spec->type, val);
                    if (ec) {

                        error = ec;
                        return out;
                    }
                    out.values[current_spec->name] = val;

                } else
                    out.values[current_spec->name] = true;       // bool flag: presence means true

            } else {

                // positional argument
                auto it = positional_specs.find(pos);
                if (it == positional_specs.end()) {

                    error = make_error_code(arg_error::unknown_argument);
                    return out;
                }
                current_spec = it->second;
                value val;
                auto ec = convert_value(arg, current_spec->type, val);
                if (ec) {

                    error = ec;
                    return out;
                }
                out.values[current_spec->name] = val;
                out.positional_order.push_back(current_spec->name);
                ++pos;
            }
        }

        // Check required arguments
        for (const auto& spec : specs) {

            if (spec.required && out.values.find(spec.name) == out.values.end()) {

                error = make_error_code(arg_error::missing_required);
                return out;
            }
        }

        error = make_error_code(arg_error::success);
        return out;
    }

    
    std::vector<std::string> tokenize_string(const std::string& cmd) {

        std::vector<std::string> tokens;
        std::stringstream ss(cmd);
        std::string token;
        while (ss >> token)
            tokens.push_back(token);

        return tokens;
    }

    // CLASS IMPLEMENTATION ============================================================================================

    // CLASS PUBLIC ====================================================================================================

    // CLASS PROTECTED =================================================================================================

    // CLASS PRIVATE ===================================================================================================

}

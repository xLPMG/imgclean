#ifndef IMGCLEAN_PROCESSORS_PROCESSOR_REGISTRY_HPP
#define IMGCLEAN_PROCESSORS_PROCESSOR_REGISTRY_HPP

#include <algorithm>
#include <cctype>
#include <string>
#include <unordered_map>
#include <vector>

#include "imgclean/GSImage.hpp"
#include "imgclean/processors/processors.hpp"

namespace imgclean
{
namespace processors
{

using ApplyProcessorFn = imgclean::GSImage (*)(const imgclean::GSImage&);

//! Normalizes a string to lowercase
//! @param s Input string
//! @return Lowercase version of the input string
inline std::string normalize(std::string s)
{
	std::transform(s.begin(), s.end(), s.begin(),
	               [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
	return s;
}

//! Gets the image processor registry containing the name of the approach (key)
//! mapped to the corresponding apply function (value).
//! @return A constant reference to the registry map
inline const std::unordered_map<std::string, ApplyProcessorFn>& registry()
{
	static const std::unordered_map<std::string, ApplyProcessorFn> kRegistry = {
		{"bradley-roth", &imgclean::processors::BradleyRothProcessor::apply},
		{"bataineh",     &imgclean::processors::BatainehProcessor::apply   },
		{"niblack",      &imgclean::processors::NiblackProcessor::apply    },
		{"sauvola",      &imgclean::processors::SauvolaProcessor::apply    },
		{"nick",         &imgclean::processors::NickProcessor::apply       },
	};
	return kRegistry;
}

//! Retrieves the processor function for a given approach name
//! @param approach Name of the processing approach
//! @return Pointer to the corresponding processor function, or nullptr if not found
inline ApplyProcessorFn get_processor(const std::string& approach)
{
	const auto& reg = registry();
	const auto it   = reg.find(normalize(approach));
	return it == reg.end() ? nullptr : it->second;
}

//! Lists all supported processing approaches
//! @return A vector of supported approach names
inline std::vector<std::string> get_supported()
{
	std::vector<std::string> keys;
	keys.reserve(registry().size());
	for (const auto& kv : registry())
	{
		keys.push_back(kv.first);
	}
	return keys;
}

} // namespace processors
} // namespace imgclean

#endif // IMGCLEAN_PROCESSORS_PROCESSOR_REGISTRY_HPP

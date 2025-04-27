
#include <algorithm>
#include <vector>
#include <map>
#include <set>
#include <string>
#include <cstring>
#include <fstream>
#include <iostream>
#include <format>


static inline const char *advance_past(const char *input, char mark) {
	const char *p = strchr(input, mark);
	if (p)
		return p + 1;
	return input + strlen(input);
}


int main(int argc, const char **argv) {
	if (argc != 2) {
		std::cerr <<
			"Usage:\n"
			"MSVC_sln_cleaner <solution.sln>\n";
		return 1;
	}

	std::string slnPath = argv[1];

	std::vector<std::string> slnTxt;
	std::set<std::string> slnKnownUuids;
	std::set<std::string> slnKnownNames;
	std::set<std::string> slnKnownPaths;
	bool skip_until_endProject = false;
	bool has_modified = false;


	{
		std::ifstream inFile(slnPath, std::ios::in);
		if (!inFile) {
			std::cerr <<
				"ERROR: Failed to open/read SLN file: " << slnPath << "\n";
			return 1;
		}

		while (inFile.good()) {
			std::string line;
			std::getline(inFile, line);

			/*
			* Example input lines:
			*
			* Project("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}") = "NTFS ADS Viewer", "..\..\..\thirdparty\DirScanner\NTFS ADS Viewer\NTFS ADS Viewer.vcxproj", "{28E9E026-5E87-4248-BA0B-07813DCD8DB1}"
			*    ...blurb...
			* EndProject
			*
			* and:
			*
			* Global
			*    ...blurb...
			* EndGlobal
			*/

			if (line.starts_with("Project(\"{")) {
				// we are interested in these: they contain name, path and UUID info which is checked by MSVC and our tools play a little fast & loose with these, so...  ;-)
				const char *input = line.c_str();

				input = advance_past(input, '{');
				int n = strcspn(input, "}");
				std::string_view prjTypeUuid(input, n);
				input += n;
				input = advance_past(input, '=');
				input = advance_past(input, '"');
				n = strcspn(input, "\"");
				std::string_view prjName(input, n);
				input += n;
				input = advance_past(input, ',');
				input = advance_past(input, '"');
				n = strcspn(input, "\"");
				std::string_view prjPath(input, n);
				input += n;
				input = advance_past(input, '{');
				n = strcspn(input, "}");
				std::string_view prjIdentifierUuid(input, n);
				input += n;
				input = advance_past(input, '"');

				std::string s(prjName);
				if (slnKnownNames.contains(s)) {
					// duplicate project name: ignore this project section.
					skip_until_endProject = true;
					has_modified = true;
				}
				slnKnownNames.insert(s);

				s = prjPath;
				if (slnKnownPaths.contains(s)) {
					// duplicate project name: ignore this project section.
					skip_until_endProject = true;
					has_modified = true;
				}
				slnKnownPaths.insert(s);

				bool uniqued = false;
				s = prjIdentifierUuid;
				if (slnKnownUuids.contains(s)) {
					// duplicate UUID: re-assign a unique value for this value...
					auto l = s.length();
					for (int i = 1; i < 8 && !uniqued; i++) {
						for (int v = 0; v < 16; v++) {
							const char c = "0123456789ABCDEF"[v];
							s[l - i] = c;
							if (!slnKnownUuids.contains(s)) {
								uniqued = true;
								break;
							}
						}
					}
				}
				slnKnownUuids.insert(s);

				if (uniqued) {
					line = std::format("Project(\"{{{}}}\") = \"{}\", \"{}\", \"{{{}}}\"{}", prjTypeUuid, prjName, prjPath, s, input);
					has_modified = true;
				}
			}

			if (!skip_until_endProject) {
				slnTxt.push_back(line);
			} else if (line.starts_with("EndProject")) {
				skip_until_endProject = false;
			}
		}
	}

	if (has_modified) {
		std::cout << "SLN has been cleaned; rewriting...\n";

		std::ofstream outFile(slnPath, std::ios::out | std::ios::trunc);
		if (!outFile) {
			std::cerr <<
				"ERROR: Failed to rewrite SLN file: " << slnPath << "\n";
			return 1;
		}

		for (auto& l : slnTxt) {
			outFile << l << "\n";
		}

		std::cout << "SLN rewritten!\n";
	} else {
		std::cout << "SLN seems clean; nothing had to be done.\n";
	}

    return 0;
}

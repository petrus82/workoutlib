import re
import argparse
from pathlib import Path


# Table to replace externally defined symbols with values
replacement = {
    "FIT_ENUM_INVALID": "0xFF",
    "FIT_UINT8_INVALID": "0xFF",
    "FIT_UINT8Z_INVALID": "0x00",
    "FIT_UINT16_INVALID": "0xFFFF",
    "FIT_UINT32_INVALID": "0xFFFFFFFF",
    "FIT_UINT32Z_INVALID": "0x00000000"
}

def resolve_symbol(name: str) -> str:
    if name.strip() in replacement:
        return replacement[name.strip()]
    return name.strip()

def skip_include_guards(line: str) -> bool:
    s = line.strip()
    if s == ("#if !defined(FIT_PROFILE_HPP)") or s == ("#define FIT_PROFILE_HPP") or s == ("#endif // !defined(FIT_PROFILE_HPP)"):
        return True
    return False

def main():
    parser = argparse.ArgumentParser(
        description="Convert C typedefs and #define blocks into C++20 module compatible code."
    )
    parser.add_argument(
        "input",
        help="Path to the input header file (e.g., fit_profile.hpp)."
    )
    parser.add_argument(
        "output",
        help="Path to the output module file (e.g., fit_profile.cppm)."
    )

    args = parser.parse_args()
    input_path = Path(args.input)
    output_path = Path(args.output)

    content = input_path.read_text(encoding="utf-8")
    lines = content.splitlines(keepends=False)  # keepends=False so we output newline separately

    start_marker = "#if !defined(FIT_CPP_INCLUDE_C) // Types defined in fit_product.h if including C code."
    end_marker   = "#endif // !defined(FIT_CPP_INCLUDE_C)"

    # Find the start and end of the region to rewrite
    start_idx = None
    end_idx   = None

    for i, line in enumerate(lines):
        if line.strip() == start_marker:
            start_idx = i
        if start_idx is not None and line.strip() == end_marker:
            end_idx = i
            break

    # Split into three parts
    before = lines[:start_idx] if start_idx is not None else lines
    after  = lines[end_idx + 1:] if end_idx is not None else []

    # ============ Parse the inner region and generate module content ============

    typedef_re = re.compile(r"^\s*typedef\s+(.+?)\s+([A-Za-z_]\w*)\s*;\s*$")
    define_re  = re.compile(r"^\s*#define\s+([A-Za-z_]\w*)\s+(.+?)(?:\s*//\s*(.*))?\s*$")
    cast_re    = re.compile(r"^\(\(\s*([A-Za-z_]\w*)\s*\)\s*(.+)\)$")
    single_cast_re = re.compile(r"^\(\s*([A-Za-z_]\w*)\s*\)\s*(.+)$")

    def strip_casts(value: str, alias: str) -> str:
        v = value.strip()
        while True:
            m = cast_re.match(v)
            if m and m.group(1) == alias:
                v = m.group(2).strip()
                continue
            m = single_cast_re.match(v)
            if m and m.group(1) == alias:
                v = m.group(2).strip()
                continue
            break
        return v

    blocks = []
    current = None

    # If we found the region, parse only that; otherwise treat the whole file as “inside”
    region_lines = lines[start_idx + 1:end_idx] if start_idx is not None and end_idx is not None else []

    for raw_line in region_lines:
        line = raw_line.strip()

        m = typedef_re.match(raw_line)
        if m:
            src_type, alias = m.groups()
            current = {
                "src_type": src_type.strip(),
                "alias":    alias.strip(),
                "defines":  [],
            }
            blocks.append(current)
            continue

        m = define_re.match(raw_line)
        if m and current is not None:
            name, value, comment = m.groups()

            value_clean = resolve_symbol(value.strip())

            current["defines"].append(
                {
                    "name":    name.strip(),
                    "value":   value_clean,
                    "comment": comment.strip() if comment else None,
                }
            )
            if name.strip().endswith("_COUNT"):
                current = None
            continue

    # Generate the module content for the middle block
    middle = []
    if blocks:
        # Put the C++20 module line at the start of this block
        middle.append("export module fitprofile;")
        middle.append("")

        for block in blocks:
            alias = block["alias"]
            src_type = block["src_type"]
            middle.append(f"export using {alias} = ::{src_type};")
            middle.append("")

            for d in block["defines"]:
                name = d["name"]
                value = strip_casts(d["value"], alias)
                comment = d["comment"]
                if comment:
                    middle.append(
                        f"export constexpr {alias} {name}{{ {value} }}; // {comment}"
                    )
                else:
                    middle.append(
                        f"export constexpr {alias} {name}{{ {value} }};"
                    )
            middle.append("")

    # ============ Reconstruct the full file ============

    out_lines = ["module;"]

    # Lines before typedef blocks
    # Only add if not include guards
    for line in before:
        stripped = line.strip()
        if skip_include_guards(stripped):
            continue
        out_lines.append(line)

    if before and middle:
        out_lines.append("")  # keep some spacing

    # Generated module block (middle)
    out_lines.extend(middle)
    if middle and after:
        out_lines.append("")

    # Lines after #endif (or entire file if markers not found)
    for line in after:
        stripped = line.strip()
        if stripped == "namespace fit":
            out_lines.append("export extern \"C++\" namespace fit")
            continue
        if skip_include_guards(stripped):
            continue
        out_lines.append(line)

    # Write back (with consistent newlines)
    result = "\n".join(out_lines) + "\n"
    output_path.write_text(result, encoding="utf-8")
    print(f"Wrote {output_path}")


if __name__ == "__main__":
    main()
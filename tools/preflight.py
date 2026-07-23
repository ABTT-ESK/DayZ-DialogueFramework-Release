"""Static checks to run before packaging. You cannot compile EnforceScript
outside the game, so this is the only safety net -- every check here exists
because something in this list once cost a failed build."""

import glob
import os
import re
import sys

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..")

# EnforceScript keywords and engine calls that look like a method call but are
# not one of ours.
KNOWN_EXTERNAL = {
    "if", "for", "foreach", "while", "switch", "return", "super", "new",
    "delete", "case", "Print", "Cast", "Insert", "Remove", "Clear", "Count",
    "Get", "Set", "Find", "ToString", "Length", "Substring", "IndexOf",
    "sizeof", "typename", "thread",
    # Engine globals
    "FileExist", "FindFile", "FindNextFile", "CloseFindFile", "OpenFile",
    "CopyFile", "DeleteFile", "MakeDirectory", "FPrintln", "FPrint",
    "CloseFile", "FGets", "GetFileAttr", "GetGame", "GetDayZGame",
    "ARGB", "Vector", "ErrorEx", "CallLater", "GetCommandModifier_Additives",
    "GetUApi", "GetYearMonthDay", "GetHourMinuteSecond",
    # Inherited from base classes we extend (UIScriptedMenu, CF modules,
    # Managed). This checker only sees our own files, so anything the engine
    # or a dependency declares has to be listed here.
    "CloseMenu", "SetFocus", "RPCSingleParam", "EnableInvokeConnect",
    # Community Framework and Expansion
    "CF_RegisterModule", "Expansion_CreateRPC", "ExpansionStatic",
    "Expansion_EnableRPCManager", "Expansion_RegisterClientRPC",
}


def strip_noise(text):
    text = re.sub(r"//.*", "", text)
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.S)
    return re.sub(r'"(\\.|[^"\\])*"', '""', text)


def check_braces(path, src):
    stripped = strip_noise(src)
    if stripped.count("{") != stripped.count("}"):
        return ["braces unbalanced (%d open, %d close)"
                % (stripped.count("{"), stripped.count("}"))]
    return []


def check_multiline(path, src):
    issues = []
    lines = src.split("\n")
    for index, line in enumerate(lines):
        nxt = ""
        if index + 1 < len(lines):
            nxt = lines[index + 1].strip()
        if nxt.startswith(("||", "&&", "?")):
            issues.append("line %d: expression wraps onto the next line"
                          % (index + 1))
    return issues


def check_ternary(path, src):
    issues = []
    for index, line in enumerate(src.split("\n")):
        code = strip_noise(line)
        if re.search(r"\?[^?:]*:", code):
            issues.append("line %d: looks like a ternary" % (index + 1))
    return issues


def check_ifdef(path, src):
    opens = len(re.findall(r"^\s*#ifdef|^\s*#ifndef", src, re.M))
    closes = len(re.findall(r"^\s*#endif", src, re.M))
    if opens != closes:
        return ["#ifdef/#endif mismatch (%d open, %d close)" % (opens, closes)]
    return []


def check_rpc_symmetry(path, src):
    issues = []
    for match in re.finditer(r"class (\w+)", src):
        name = match.group(1)
        body = src[match.start():]
        send = re.search(r"void OnSend\(ScriptRPC rpc\)(.*?)\n\t\}", body, re.S)
        recv = re.search(r"bool OnRecieve\(ParamsReadContext ctx\)(.*?)\n\t\}",
                         body, re.S)
        if not send or not recv:
            continue
        writes = re.findall(r"rpc\.Write\(|WriteList\(rpc,", send.group(1))
        reads = re.findall(r"ctx\.Read\(|ReadList\(ctx,", recv.group(1))
        if len(writes) != len(reads):
            issues.append("%s: OnSend writes %d field(s), OnRecieve reads %d"
                          % (name, len(writes), len(reads)))
    return issues


#! A declaration is: optional modifiers, a RETURN TYPE, whitespace, the name,
#! then the bracket. Requiring a real type token matters -- a looser pattern
#! matches the indentation on a call site and treats every call as its own
#! declaration, which makes this whole check silently pass.
DECLARATION = re.compile(
    r"^[ \t]*"
    r"(?:(?:static|protected|private|override|proto|native|ref|const|autoptr)"
    r"[ \t]+)*"
    r"(?:void|bool|int|float|string|vector|typename"
    r"|[A-Za-z_]\w*(?:<[^>]+>)?(?:\[\])?)"
    r"[ \t]+"
    r"([A-Za-z_]\w*)[ \t]*\(",
    re.M)


def collect_methods(sources):
    """Every method this mod declares, across all its script files."""
    declared = set()
    for src in sources.values():
        for match in DECLARATION.finditer(src):
            declared.add(match.group(1))
    return declared


def check_calls_resolve(sources):
    """Catches a method that is called but no longer defined -- the exact
    failure that follows deleting or renaming a block of code."""
    declared = collect_methods(sources)
    issues = []
    for path, src in sources.items():
        for index, line in enumerate(src.split("\n")):
            code = strip_noise(line)
            for match in re.finditer(r"(?<![\w.])(\w+)\s*\(", code):
                name = match.group(1)
                if name in KNOWN_EXTERNAL or name in declared:
                    continue
                # Only flag bare calls, not Something.Method() or declarations
                before = code[:match.start()].rstrip()
                if before.endswith("."):
                    continue
                if re.search(r"[\w>]\s*$", before):
                    continue
                issues.append("%s line %d: call to undefined '%s'"
                              % (os.path.basename(path), index + 1, name))
    return issues


def main():
    patterns = [
        os.path.join(ROOT, "Scripts", "**", "*.c"),
        os.path.join(ROOT, "GUI", "layouts", "*.layout"),
    ]
    paths = []
    for pattern in patterns:
        paths.extend(glob.glob(pattern, recursive=True))

    sources = {}
    for path in paths:
        with open(path, encoding="utf-8") as handle:
            sources[path] = handle.read()

    failures = []
    for path, src in sources.items():
        name = os.path.relpath(path, ROOT)
        for check in (check_braces, check_multiline, check_ifdef):
            for issue in check(path, src):
                failures.append("%s: %s" % (name, issue))
        if path.endswith(".c"):
            for check in (check_ternary, check_rpc_symmetry):
                for issue in check(path, src):
                    failures.append("%s: %s" % (name, issue))

    script_sources = {}
    for path, src in sources.items():
        if path.endswith(".c"):
            script_sources[path] = src
    failures.extend(check_calls_resolve(script_sources))

    if failures:
        print("PRE-FLIGHT FAILED\n")
        for failure in failures:
            print("  " + failure)
        return 1

    print("pre-flight passed: %d file(s) checked" % len(sources))
    return 0


if __name__ == "__main__":
    sys.exit(main())

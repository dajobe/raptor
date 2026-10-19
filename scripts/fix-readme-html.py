#!/usr/bin/env python3
#
# Prepare README.html for conversion to the GitHub README.md
#
# Usage: python3 fix-readme-html.py README.html | pandoc --from=html --to=gfm ...
#
# README.html is written for the website where relative links resolve
# against the project pages.  The Markdown copy is read on GitHub, so
# relative links are made absolute, the centered author byline heading
# becomes a paragraph and line breaks become paragraph breaks.
#
# (C) Copyright 2026 Dave Beckett
#

import os
import re
import sys

WEBSITE_BASE = "https://librdf.org/raptor/"


def absolutize_href(match):
    """
    A replacer function for re.sub to make relative hrefs absolute.
    """
    href = match.group(1)
    if href.startswith("#") or re.match(r"[A-Za-z][A-Za-z0-9+.-]*:", href):
        return match.group(0)
    return f'href="{WEBSITE_BASE}{href}"'


def main():
    """
    Main function to read README.html and write the fixed HTML to stdout.
    """
    progname = os.path.basename(sys.argv[0])

    if len(sys.argv) != 2:
        print(f"USAGE: {progname} README.html", file=sys.stderr)
        sys.exit(1)

    try:
        with open(sys.argv[1], encoding="utf-8") as in_f:
            document = in_f.read()
    except OSError as e:
        print(f"{progname}: Cannot read {sys.argv[1]} - {e}", file=sys.stderr)
        sys.exit(1)

    document = re.sub(r'href="([^"]*)"', absolutize_href, document)
    document = re.sub(
        r'<h2 style="text-align:center">(.*?)</h2>',
        r"<p>\1</p>",
        document,
        flags=re.DOTALL,
    )
    # Pandoc versions write Markdown hard line breaks differently
    # (trailing spaces or a backslash) so use paragraphs instead.
    document = re.sub(r"<br\s*/?>", "</p>\n<p>", document)

    sys.stdout.write(document)


if __name__ == "__main__":
    main()

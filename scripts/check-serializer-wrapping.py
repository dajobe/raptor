#!/usr/bin/env python3
"""Check serializer line wrapping and preservation of RDF terms.

The caller supplies the input file, formats, serializer and wrapping width.
RDF terms are compared as sorted N-Triples, so fixtures must not use blank nodes.
"""

import argparse
from pathlib import Path
import subprocess
import sys
import tempfile


def serialize(rapper, source, input_format, output_format, width=None):
    """Run rapper and return its output without decoding literal bytes."""
    command = [str(rapper), "-q", "-i", input_format, "-o", output_format]
    if width is not None:
        command.extend(["-f", "lineLength={}".format(width)])
    command.extend([str(source), "-"])
    return subprocess.run(command, stdout=subprocess.PIPE, check=True).stdout


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("rapper", type=Path)
    parser.add_argument("source", type=Path)
    parser.add_argument("--input-format", required=True)
    parser.add_argument("--serializer", required=True)
    parser.add_argument("--output-parser", required=True)
    parser.add_argument("--line-length", type=int, required=True)
    args = parser.parse_args()
    rapper = args.rapper.resolve()
    source = args.source.resolve()
    if args.line_length <= 0:
        parser.error("--line-length must be positive")

    expected = sorted(
        serialize(rapper, source, args.input_format, "ntriples").splitlines()
    )
    with tempfile.TemporaryDirectory(prefix="raptor-line-length-") as directory:
        outputs = {}
        for width in (args.line_length, 0):
            output = serialize(
                rapper, source, args.input_format, args.serializer, width
            )
            outputs[width] = output
            path = Path(directory) / "output-{}".format(width)
            path.write_bytes(output)
            actual = serialize(rapper, path, args.output_parser, "ntriples")
            if sorted(actual.splitlines()) != expected:
                raise ValueError(
                    "{}: lineLength={} changed RDF terms".format(args.serializer, width)
                )
        if len(outputs[args.line_length].splitlines()) <= len(outputs[0].splitlines()):
            raise ValueError(
                "{}: lineLength={} did not wrap output".format(
                    args.serializer, args.line_length
                )
            )
    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except (OSError, subprocess.CalledProcessError, ValueError) as error:
        print(error, file=sys.stderr)
        sys.exit(1)

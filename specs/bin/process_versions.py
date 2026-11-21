#!/usr/bin/env python3
# Copyright 2015-2026 The Khronos Group Inc.
# SPDX-License-Identifier: Apache-2.0

"""Process version.* files to produce version info, author info, and change log.

For example, version 1.0 Revision 4 should be described in file version.1.0.4

Output files are:
    auto.version.adoc      - Version info (now just an asciidoc attribute with the version string)
    auto.intro.author.adoc - Author info.
    auto.changes.adoc      - Collected change log, ordered by version.
"""

import os
import re


def key_from_version_filename(name):
    """Returns a list of version number components for the given version
    filename.   The name should begin with 'version.' and then be followed
    a sequence of decimal integers separated by '.'.

    Returns:  The list of integers in the name string, in order.
    """
    parts = name.split('.')
    if parts[0] != 'version':
        raise RutimeException('%s does not begin with "version."' % name)
    return [ int(i) for i in parts[1:] ]


def format_version_from_name(versionfile):
    components = key_from_version_filename(versionfile)
    parts = [str(components[0]), '.']
    if components[1] == 0:
        parts.append('00') # John likes it this way. :-)
    else:
        parts.append(str(components[1]))
    parts.append(', Revision %d' % (components[2]))
    parts.append(': Unified')
    return ''.join(parts)


def get_version_files_in_order(dir):
    """Returns the list of version file names in the given directory,
    ordered by version number"""
    files = [ f for f in os.listdir(dir) if f.startswith("version.") ]
    return sorted(files, key=key_from_version_filename)


def generate_asciidoc_front_matter(out, versionfile):
    """Writes front matter deduced from the given version file
    into the specified output file handle."""
    with open(versionfile, 'r') as f:
        out.write('%s\n' % (format_version_from_name(versionfile)))

def generate_asciidoc_author(out, versionfile):
    """Writes front matter deduced from the given version file
    into the specified output file handle."""
    with open(versionfile, 'r') as f:
        authors = []
        for line in f:
            if line.startswith('author: '):
                authors.append('  * ' + line[8:].strip())
        # assume last is current
        authors[-1] = authors[-1] + ' (current)'
        out.write('%s\n' % ('\n'.join(authors)))


class Author:
    """An Author object holds an author first name, surname, and
    organizational affiliation.  The first name is always just the
    first word of the name.  The surname may contain middle names and
    initials.
    """
    def __init__(self, name_and_affiliation):
        """Initializes an Author object from a string such as
          ' John Kessenich, Google\n' """
        expr = re.compile(r'\s*(\S+)\s*(.*),\s+(.*)')
        match = expr.match(name_and_affiliation.strip())
        self.first_name = match.group(1)
        self.surname = match.group(2)
        self.org = match.group(3)


def generate_docbook_authors(out, versionfile):
    """Writes author information in DocBook format into the specified
    output file handle."""
    authors = []
    with open(versionfile, 'r') as f:
        for line in f:
            if line.startswith('author: '):
                authors.append(Author(line[8:]))
    out.write("<authorgroup>")

    # Note.  DocBook allows many kinds of affiliations.
    # The first versions of SPIRV-docbook.xml used orgname for JohnK
    # and orgdiv for BoazO.  Choosing corpname since both are actually
    # just corporations.
    # Also, this code matches the original SPIRV-docbook.xml by putting
    # both surname and organization in the surname element.
    for a in authors:
        out.write("""
    <author>
      <firstname>{first_name}</firstname>
      <surname>{surname}, {org}</surname>
      <affiliation>
        <corpname>{org}</corpname>
      </affiliation>
    </author>""".format(first_name=a.first_name,
                        surname=a.surname, org=a.org))
    out.write("\n</authorgroup>\n")


def generate_changelog(out, version_files):
    """Writes the change log by concatenating the change log portion
    of the version files.

    Args:
        out: output file stream
        version_files: names of the version.* files, ordered by version
           number
    """
    for version_file in version_files:
        out.write('\n')
        with open(version_file, 'r') as f:
            in_changes = False
            for line in f:
                if in_changes:
                    out.write(line)
                elif line.startswith('changes:'):
                    in_changes = True


def main():
    files = get_version_files_in_order(os.getcwd())

    # The front matter only depends on the highest version file.
    with open('auto.version.adoc', 'w') as intro_file:
        generate_asciidoc_front_matter(intro_file, files[-1])

    # Editor.
    with open('auto.intro.author.adoc', 'w') as intro_file:
        generate_asciidoc_author(intro_file, files[-1])

    with open('auto.changes.adoc', 'w') as changelog:
        generate_changelog(changelog, files)

if __name__ == '__main__':
    main()

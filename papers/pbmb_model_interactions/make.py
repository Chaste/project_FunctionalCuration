#!/usr/bin/env python

"""Copyright (c) 2005-2015, University of Oxford.
All rights reserved.

University of Oxford means the Chancellor, Masters and Scholars of the
University of Oxford, having an administrative office at Wellington
Square, Oxford OX1 2JD, UK.

This file is part of Chaste.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.
 * Neither the name of the University of Oxford nor the names of its
   contributors may be used to endorse or promote products derived from this
   software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
"""

# Script to make processing LaTeX files easy


import os
import re
import sys
import optparse

PAPER, TALK, POSTER = 0, 1, 2

usage = """usage: %prog [options] [<LaTeX file> ...]

If arguments are supplied, these are taken to be the
names of LaTeX input files.  Otherwise all .tex files
in DIR are processed."""
parser = optparse.OptionParser(usage=usage)
parser.add_option('-d', '--dir',
                  default='.',
                  help="process files in directory DIR [default: %default]")
parser.add_option('--no-convert', dest='convert', default=True,
                  action='store_false',
                  help="do not convert any graphics files to .eps/.pdf")
parser.add_option('--remove', dest='remove', default=False,
                  action='store_true',
                  help="remove .eps files when done")
parser.add_option('--ps', dest='ps', default=False,
                  action='store_true',
                  help="create a .ps file")
parser.add_option('--no-pdf', dest='pdf', default=True,
                  action='store_false',
                  help="do not create a .pdf file")
parser.add_option('--bib', dest='bib', default=False,
                  action='store_true',
                  help="include a bibliography using BibTeX")
parser.add_option('--paper', dest='type', default=PAPER,
                  action='store_const', const=PAPER,
                  help="use settings to generate a paper [default]")
parser.add_option('--talk', dest='type', 
                  action='store_const', const=TALK,
                  help="use settings to generate a (prosper) presentation.  "
                  "Implies --pdf and --ps")
parser.add_option('--poster', dest='type',
                  action='store_const', const=POSTER,
                  help="use settings to generate a poster (same as --paper)")
parser.add_option('--acm', dest='acm',
                  action='store_true', default=False,
                  help="use PS&PDF creation flags for ACM conference proceedings")
parser.add_option('-v', '--trace', '--verbose',
                  dest='trace', action='store_true', default=False,
                  help="show all commands executed")
parser.add_option('-p', '--use-pdflatex',
                  action='store_true', default=False,
                  help="use pdflatex if only generating PDF output.  "
                  "If automatic figure conversion is on, .eps files will be "
                  "converted to .pdf using epstopdf.")
parser.add_option('-f', '--figure-dir', default=None,
                  help="expect figures in FIGURE_DIR [default: DIR].  "
                  "If a relative path is given, it will be resolved relative "
                  "to DIR.")
options, ltexfiles = parser.parse_args()

##print options

if options.type == TALK:
  options.ps = True
  options.pdf = True

if options.acm:
  options.ps = True
  options.pdf = True
  options.type = PAPER

if options.figure_dir is None:
  options.figure_dir = options.dir

if options.use_pdflatex and options.ps:
  # We need to run normal latex to generate .ps
  options.use_pdflatex = False
if options.use_pdflatex:
  # Use pdflatex
  latex = 'pdflatex '
else:
  latex = 'latex '

# Change into directory to process
if not os.path.isdir(options.dir):
  parser.error("Supplied directory `" + options.dir + "' is not a directory.")
os.chdir(options.dir)

# Find files to process
if len(ltexfiles) > 0:
  for i, texfile in enumerate(ltexfiles):
    if os.path.splitext(texfile)[1] != '.tex':
      ltexfiles[i] = texfile + '.tex'
  texfiles = " ".join(ltexfiles)
else:
  texfiles = "*.tex"
  import glob
  ltexfiles = glob.glob(texfiles)
print "*** Will process",texfiles


#
# Helper functions
#

def do_cmd(cmd):
  """Execute the given command.

  If tracing is on, also print the command."""
  if options.trace:
    print
    print ">>>", cmd
    print
  os.system(cmd)


natbib_re = re.compile(r"\(natbib\)\s+Rerun to get citations correct.")
def needs_latex_rerun(texfile):
  """Parse log file to determine if we need to re-run LaTeX on texfile."""
  logfile = os.path.splitext(texfile)[0] + '.log'
  fp = file(logfile)
  rerun = False
  if fp:
    for line in fp:
      if line.startswith('LaTeX Warning: Label(s) may have changed. Rerun'):
        rerun = True
        break
      m = natbib_re.match(line)
      if m:
        rerun = True
        break
    fp.close()
  return rerun


def newer(fileA, fileB):
  """Is fileA newer than fileB?"""
  return os.stat(fileA).st_mtime > os.stat(fileB).st_mtime


def convert_latex_fig(filepath, to_path):
  """Convert an image defined using pstricks into an .eps file.

  Requires the existence of a file template.tex in the current directory,
  which contains the line
    \input{##file##}
  to use as a document template.  The file latexfig.tex will be written,
  with the above line replaced as appropriate.
  """
  # Write a LaTeX document based on the template
  base, ext = os.path.splitext(filepath)
  docfile = 'latexfig.tex'
  template = open('template.tex', 'r')
  doc = open(docfile, 'w')
  for line in template:
    doc.write(line.replace('##file##', base))
  template.close()
  doc.close()
  # Run latex on it
  do_cmd('latex ' + docfile)
  # Generate eps
  do_cmd('dvips -E -o ' + base+'.eps'
         + ' ' + os.path.splitext(docfile)[0] + '.dvi')

def process_figures(remove=False, latex=False):
  """Find figure files and convert them or remove converted files."""
  def make_func(prog):
    """Convert a program name into a python function which calls it."""
    if prog == 'epstopdf':
      to_pre = ' -o='
    else:
      to_pre = ''
    return lambda img_path, to_path: do_cmd(prog + " " + img_path +
                                            to_pre + to_path)
  # Conversion functions
  funcs = {'.tex': convert_latex_fig,
           '.eps': make_func('epstopdf'),
           '.ps': make_func('epstopdf'),
           '.gif': make_func('convert'),
           '.jpg': make_func('convert'),
           '.png': make_func('convert')}
  if latex:
    from_exts = ['.tex']
    to_exts = '.eps'
  elif options.use_pdflatex:
    from_exts = ['.eps', '.ps', '.gif']
    to_exts = {'.eps': '.pdf', '.ps': '.pdf', '.gif': '.png'}
  else:
    from_exts = ['.jpg', '.gif', '.png']
    to_exts = '.eps'
  for img in os.listdir(options.figure_dir):
    base, ext = os.path.splitext(img)
    if ext in from_exts:
      if type(to_exts) == type(''):
        to_ext = to_exts
      else:
        to_ext = to_exts[ext]
      to_path = os.path.join(options.figure_dir, base + to_ext)
      img_path = os.path.join(options.figure_dir, img)
      if remove:
        # Only remove if 'converted' file is newer than source
        if os.path.isfile(to_path) and newer(to_path, img_path):
          do_cmd("/bin/rm -f " + to_path)
      else:
        # Check if already converted
        if not os.path.isfile(to_path) or newer(img_path, to_path):
          print "* Converting", img, "to", to_ext
          funcs[ext](img_path, to_path)

#
# Do stuff!
#

# Convert any LaTeX figures, provided a figure dir is given
if options.figure_dir != options.dir:
  process_figures(latex=True)

# Convert graphics
if options.convert:
  process_figures()

# Bibliography
if options.bib:
  for texfile in ltexfiles:
    do_cmd(latex + texfile)
    do_cmd("bibtex " + os.path.splitext(texfile)[0])

# Process LaTeX files
for texfile in ltexfiles:
  do_cmd(latex + texfile)
  if needs_latex_rerun(texfile):
    do_cmd(latex + texfile)
  if needs_latex_rerun(texfile):
    do_cmd(latex + texfile)
  
# Create ps if requested
if options.ps:
  print "** Creating .ps file"
  if options.type == TALK:
    dvips_args = ' -t a4 -Ppdf -G0 '
  elif options.acm:
    dvips_args = ' -P cmz -t letter '
  else:
    dvips_args = ''
  cmd_template = "dvips" + dvips_args + " -o %s.ps %s.dvi"
  for texfile in ltexfiles:
    do_cmd(cmd_template % (texfile[:-4], texfile[:-4]))

# Create pdf
if options.pdf and not options.use_pdflatex:
  print "** Creating .pdf file"
  if options.type == TALK:
    for texfile in ltexfiles:
      do_cmd("ps2pdf -dPDFsettings=/prepress -dAutoFilterColorImages=false -dColorImageFilter=/FlateEncode " + texfile[:-3] + "ps")
  else:
    if len(ltexfiles) == 0:
      do_cmd("dvipdf *.dvi")
    else:
      for texfile in ltexfiles:
        if options.acm:
          do_cmd("ps2pdf " + texfile[:-3] + "ps")
        else:
          do_cmd("dvipdf " + texfile[:-3] + "dvi")

# Remove converted figure files
if options.remove:
  process_figures(remove=True)

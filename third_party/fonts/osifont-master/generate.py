import fontforge

import os

"""
 Generate all osifont font files with appropriate license
"""

GPL_FONT_COPYRIGHT = """
Created by
Zefram Cochrane,
hikikomori82@gmail.com,
with FontForge 2.0 (http://fontforge.sf.net)

Project hosted on:
http://github.com/hikikomori82/osifont/

Distributed under GNU GPL version 3 with GPL font exception:

As a special exception, if you create a document which uses this font, and embed this font or unaltered portions of this font into the document, this font does not by itself cause the resulting document to be covered by the GNU General Public License. This exception does not however invalidate any other reasons why the document might be covered by the GNU General Public License. If you modify this font, you may extend this exception to your version of the font, but you are not obligated to do so. If you do not wish to do so, delete this exception statement from your version.
""".strip()

GPL2_FONT_COPYRIGHT ="""
Created by
Zefram Cochrane,
hikikomori82@gmail.com,
with FontForge 2.0 (http://fontforge.sf.net)

Project hosted on:
http://github.com/hikikomori82/osifont/

Distributed under GNU GPL version 2 with GPL font exception:

As a special exception, if you create a document which uses this font, and embed this font or unaltered portions of this font into the document, this font does not by itself cause the resulting document to be covered by the GNU General Public License. This exception does not however invalidate any other reasons why the document might be covered by the GNU General Public License. If you modify this font, you may extend this exception to your version of the font, but you are not obligated to do so. If you do not wish to do so, delete this exception statement from your version.
""".strip()

LGPL3_FONT_COPYRIGHT = """
Created by
Zefram Cochrane,
hikikomori82@gmail.com,
with FontForge 2.0 (http://fontforge.sf.net)

Project hosted on:
http://github.com/hikikomori82/osifont/

Distributed under GNU LGPL version 3 with GPL font exception:

As a special exception, if you create a document which uses this font, and embed this font or unaltered portions of this font into the document, this font does not by itself cause the resulting document to be covered by the GNU General Public License. This exception does not however invalidate any other reasons why the document might be covered by the GNU General Public License. If you modify this font, you may extend this exception to your version of the font, but you are not obligated to do so. If you do not wish to do so, delete this exception statement from your version.
""".strip()

#Windows 20230101 fontforge notes:
#fontforge.hooks['loadFontHook'] Does not trigger
#fontforge.hooks['newFontHook'] Not available
#font.temporary['generateFontPostHook'] Available, always called twice
#font.temporary['generateFontPreHook'] Not available
#'fontforge.font' is not an acceptable base type
#fontforge -dry script.py does not work in Python 3.10.9

class FontWrapper():
   """ Class to report on file creation """
   def __init__(self, fileName):
      # SplineFontDatabase per https://fontforge.org/docs/techref/sfdformat.html
      self.mSfdFileName = fileName
      print("Opening %s spline font database file" % fileName)
      self.mFont = fontforge.open(self.mSfdFileName)

   def setCopyright(self, copyright):
      """ Assign the copyright statement to the font. """
      self.mFont.copyright = copyright

   def _removeFile(self, fileName):
      """ Remove file on disk if it exits """
      try:
         os.remove(fileName)
      except FileNotFoundError:
         pass
      except:
         print(" Could not remove %s" % fileName)
         raise

   def generate(self, fileName):
      """ Write file and report if file write was successful. """
      # Windows 20230101 no mechanism to check that generate succeeded?
      self._removeFile(fileName)

      print(" Generating %s" % fileName)
      _fontObj = self.mFont.generate(fileName)
      #assert _fontObj is self.mFont
      msg = "FAILED to generate %s from %s" % (fileName, self.mSfdFileName)
      if os.path.exists(fileName):
         msg = " Generated %s from %s" % (fileName, self.mSfdFileName)
      print(msg)

fontObj = FontWrapper("osifont.sfd")
fontObj.setCopyright(GPL_FONT_COPYRIGHT)
fontObj.generate("osifont.ttf")
fontObj.generate("osifont.woff")

fontObj.setCopyright(GPL2_FONT_COPYRIGHT)
fontObj.generate("osifont-gpl2fe.ttf")
fontObj.generate("osifont-gpl2fe.woff")

fontObj.setCopyright(LGPL3_FONT_COPYRIGHT)
fontObj.generate("osifont-lgpl3fe.ttf")
fontObj.generate("osifont-lgpl3fe.woff")

italicFontObj = FontWrapper("osifont-italic.sfd")
# merge osifont glyphs to osifont-italic?
italicFontObj.setCopyright(LGPL3_FONT_COPYRIGHT)
italicFontObj.generate("osifont-italic.ttf")


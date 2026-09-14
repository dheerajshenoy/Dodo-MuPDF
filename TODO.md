# TODO

## This is a list of features and improvements that I want to implement in the future. The list is not exhaustive and is subject to change based on user feedback and my own priorities.

## HIGH PRIORITY

- [ ] Fix page shifting when zooming in/out
- [ ] EPUB reflow
- [ ] Text reflow for MuPDF documents
- [ ] Tab detach drag and drop to new window spawns new useless window

## MEDIUM PRIORITY

- [ ] Bookmarks/history/sessions store raw `PageLocation{pageno,x,y}` (`include/PageLocation.hpp`), which an EPUB/reflowable-document relayout invalidates. Short-term (already shipped alongside reflow): clamp to new page count on load (approximately right page, not exact position). Real fix — anchoring EPUB bookmarks to `(chapter, uri-fragment, y-fraction)` like the outline now does (see `Model::resolveOutlineNode`) — is a `PageLocation`/`BookmarkManager` schema change and belongs in its own follow-up.
- [ ] Decorate form fields
- [ ] Trim margins
- [ ] Add support for directory local config files
- [ ] Allow for command arguments
- [ ] Don't add connection to annotation when in non-annotatable mode
- [ ] Link hint lua api and then callback to lua
- [ ] Add luajit support
- [ ] Add support for embedded files in PDFs
- [ ] Underline Annotation
- [ ] Djvu Text selection
- [ ] Djvu Text search

## LUA PLUGIN IDEAS

- [ ] Equation OCR to LaTeX
- [ ] Table exporter to tex/CSV/Excel/Numpy
- [ ] Semantic search
- [ ] Finding citation from folder
- [ ] Explain selection
- [ ] Search inside math equations

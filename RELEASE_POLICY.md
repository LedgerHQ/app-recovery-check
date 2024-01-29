Release Policy
==============

Every so often a new release is tagged on the develop branch and is
assigned a vX.Y.Z version number, where X is incremented each time a release
contains breaking changes, Y is incremented on changes such as added features,
and Z is incremented if a release contains only bugfixes and other minor
changes.

Releases are tagged on the develop branch and will not be maintained separately.
Releases may be made if the amount of commits or severity of bugs justify it, or 
in the event of security issues.

The goal of a release is to provide assets such as compiled app binaries. If you 
want the newest features, just use the latest commit on the develop branch.
We try our best to keep it deployable at all times.

Releases other than the latest release are unsupported and unmaintained.

Release procedure
-----------------

While on the develop branch:

- Update the `CHANGELOG` file, adding to the previous release notes.

- Update the `Makefile` file, changing the `APPVERSION` variables

- Create a signed commit with the changes.

- Create a signed tag vX.Y.Z (the lowercase 'v' is important)

- The tag should contain a comment in the same format as the `CHANGELOG` file for that release

- Push the commit and tag to GitHub.

- Pushing the tag will trigger the "Release Workflow" GitHub Action. This action will build the application 
for all device types, package the binaries and then create a release page at https://github.com/aido/app-seed-tool/releases 

Changelog template
------------------

All notable changes to this project will be documented in this file.
 
The format is based on [Keep a Changelog](http://keepachangelog.com/)
and this project adheres to [Semantic Versioning](http://semver.org/).

Here is a template that can be used for writing the `CHANGELOG` file:

```markdown
## [X.Y.Z] - 1970-01-01
### Added
- List of new features

### Changed
- List of changed features

### Fixed
- List of bug fixes
```
The comment in the release tag follows a similar but not exactly same format as follows:
```markdown
[X.Y.Z] - 1970-01-01
Added
- List of new features

Changed
- List of changed features

Fixed
- List of bug fixes
```

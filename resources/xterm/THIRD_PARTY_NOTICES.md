# Third-party terminal assets

DD-SSH bundles the following development assets for the Qt WebEngine terminal renderer:

- `@xterm/xterm` version 5.5.0
- `@xterm/addon-fit` version 0.10.0

Both packages are distributed under the MIT license.

The license texts are included in this directory:

- `LICENSE.xterm.txt`
- `LICENSE.addon-fit.txt`

These files are bundled so DD-SSH's terminal renderer can work offline and does not need to load xterm.js from a CDN at runtime.

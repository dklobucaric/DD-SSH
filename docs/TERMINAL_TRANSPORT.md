# Terminal Transport

**Checkpoint:** dev 0.1.7.0 — Andromeda

DD-SSH treats the SSH terminal as a byte stream. Terminal output from libssh is no longer converted directly from each individual read chunk into a `QString` for xterm.js. Instead, worker output is passed as bytes, encoded to Base64 for Qt WebChannel, and decoded in the WebEngine renderer with a streaming UTF-8 `TextDecoder`.

This reduces the risk of corrupting UTF-8 characters when a multi-byte character is split across SSH reads. It also keeps the terminal transport direction compatible with future terminal/file-transfer work.

## Input path

Terminal input is queued as UTF-8 bytes in `SshShellWorker`. Large input/paste content is not artificially blocked. The worker writes the queued bytes to libssh using a partial-write-aware loop so that `ssh_channel_write()` returning less than the requested byte count does not silently drop the remaining bytes.

## Output path

```text
libssh read bytes
  -> SshShellWorker::outputReceived(QByteArray)
  -> TerminalBridge::outputBytesReceived(base64)
  -> JavaScript Uint8Array
  -> TextDecoder('utf-8', stream=true)
  -> xterm.js write()
```

The basic/fallback terminal also uses a streaming Qt UTF-8 decoder before applying its simple ANSI cleanup.

## Privacy

Diagnostic logging records lifecycle events and traffic summaries only. It must not log terminal input, terminal output, pasted text, passwords, private keys, or full JSON config content.

## Not changed

This checkpoint does not add SFTP, file manager, encryption, new config schema, or OS-level network monitoring. Session Traffic still measures DD-SSH SSH channel bytes, not global OS traffic.

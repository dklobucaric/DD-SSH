# DD-SSH dev 0.1.8.6.2 — Test case

This checkpoint fixes terminal paste newline handling. It should preserve multiline formatting when pasting YAML/config text into full-screen terminal editors such as nano.

## Test YAML

```yaml
version: "3.7"

services:
  caddy:
    container_name: caddy
    image: docker.io/library/caddy:2-alpine
    network_mode: host
    restart: unless-stopped
    volumes:
      - ./Caddyfile:/etc/caddy/Caddyfile:ro
      - caddy-data:/data:rw
      - caddy-config:/config:rw
```

## Checks

- [ ] About shows `dev 0.1.8.6.2`.
- [ ] Open SSH terminal and run `nano /tmp/dd-ssh-paste-test.yml`.
- [ ] Paste the YAML block using toolbar Paste.
- [ ] The pasted text keeps line breaks and indentation.
- [ ] nano does not show `Justified paragraph` after paste.
- [ ] Save the file and verify with `cat -n /tmp/dd-ssh-paste-test.yml`.
- [ ] Repeat using right-click paste if available.
- [ ] Repeat using Ctrl+Shift+V on Linux/Windows or Command+V on macOS.
- [ ] Single-line paste still works.
- [ ] Ctrl+C remote interrupt still works.
- [ ] File Manager queue/delete smoke still works.

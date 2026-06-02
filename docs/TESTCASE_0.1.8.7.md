# Testcase — dev 0.1.8.7

Goal: verify release/tester packaging polish without regressing the accepted terminal and File Manager baseline.

## App metadata

- [ ] About shows `dev 0.1.8.7`.
- [ ] Codename is Andromeda.
- [ ] Current phase says release/tester packaging polish for File Manager alpha.
- [ ] Milestone says Release/tester packaging polish.
- [ ] Developer line says `DD-LAB vl. Dalibor Klobučarić`.

## Documentation

- [ ] README current status mentions `dev 0.1.8.7`.
- [ ] `docs/FEATURE_AUDIT_0.1.8.7.md` exists.
- [ ] `docs/RELEASE_TESTER_PACKAGING_0.1.8.7.md` exists.
- [ ] `docs/TESTER_CHECKLIST_0.1.8.7.md` exists.
- [ ] `docs/TEST_MATRIX.md` contains the current checkpoint.

## Runtime smoke

- [ ] Terminal opens and paste still works.
- [ ] File Manager opens and queues upload/download.
- [ ] Delete local/delete remote queue confirmations preserve item order.
- [ ] Logging remains OFF by default and logs transfer metadata only when enabled.

## Packaging smoke

- [ ] Linux `.deb` filename/version is `0.1.8.7`.
- [ ] macOS `.dmg` filename/version is `0.1.8.7`.
- [ ] Windows portable ZIP filename/version is `0.1.8.7`.
- [ ] Checksums can be generated for final artifacts.

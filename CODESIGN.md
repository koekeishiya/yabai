Create *Code Signing* certificate named "yabai-cert" using *Keychain Access.app*:

* Open *Keychain Access.app*
* From *menu* select *Keychain Access* -> *Certificate Assistant* -> *Create a certificate*
* Fill the certificate form:
    * Name: yabai-cert
    * Identity Type: Self Signed Root
    * Certificate Type: Code Signing

Sign the binary:

```
codesign -fs "yabai-cert" /path/to/bin/yabai
```

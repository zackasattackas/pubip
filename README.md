# pubip
Simple command line app that retrieves the public IP of the current Windows host machine.

This was really just a weekend project for me to get some practice with C, and I was curious to see how the native Win32 API's for http requests work. That said, this is still a useful tool, albeit not a very a unique one. I used the [ipify](https://www.ipify.org/) API to get the public IP.

### Command line syntax

```
C:\> pubip [-json]
```

The only command line option is `-json`, which optionally outputs the IP address as a json object.

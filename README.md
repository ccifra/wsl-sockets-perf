# Raw Sockets Performance Testing

Testing performance of raw sockets between windows client and WSL2 Host
* Tests TCP sockets over localhost
* Tests Hypervisor sockets

## Setup

You must get the GUID of the WSL client VM by running
```
hcsdiag list
```
From an administrator powershell script

You must register your integration service with Windows

```
$friendlyName = "HV Socket Test"
$service = New-Item -Path "HKLM:\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Virtualization\GuestCommunicationServices" -Name "0000C384-facb-11e6-bd58-64006a7986d3"
$service.("ElementName", $friendlyName)
```

The GUID that we are setting in the registry is the HV_GUID_VSOCK_TEMPLATE, meaning <port>-facb-11e6-bd58-64006a7986d3
And in our case the port we are setting is 50053 (in hex in the GUID)

If you do not register the service you will get an unathorized error trying to connect

When you run the client you have to pass the VM ID GUID on the command line.

## Building the Linux Server

```
g++ linux_server.cc -o server
```

## Building the Windows Client

```
mkdir build
cd build
cmake ..
cmake --build .
```

## Run the Tests

### TCP Performance

localhost Windows
```
sockets_server
```

```
sockets_client
```

Windows to WSL

Linux
```
./server
```

Windows
```
sockets_client
```

### VSOCK Performance

Linux
```
./server -vsock
```

Windows
```
sockets_client.exe -vsock FC65AFDB-069B-4AB0-ACBF-274C34BF8593
```
Replace the GUID with the VM ID you found with `hcsdiag list`


My Results (XIDAX Machine, Windows 10, Ubuntu 20.04)
```
Windows localhost
Latency 18us, handshaked throughput 4675 MB/s, non handshaked throughput 4769.52 MB/s

WSL TCP
Latency 134us, handshaked throughput 115.6 MB/s, non handshaked throughput 127.5 MB/s

WSL VSock
Latency 56us, handshaked throughput 1323 MB/s, non handshaked throughput 1872.7 MB/s
```

## Useful Links
* https://stackoverflow.com/questions/60696166/using-the-hyper-v-sockets-between-windows-host-and-linux-guest
* https://github.com/Biswa96/wslbridge2/tree/master/samples
* https://docs.microsoft.com/en-us/virtualization/hyper-v-on-windows/user-guide/make-integration-service

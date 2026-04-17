import 'package:flutter/material.dart';
import 'package:flutter_blue_plus/flutter_blue_plus.dart';

void main() => runApp(const MaterialApp(home: BleScannerScreen()));

class BleScannerScreen extends StatefulWidget {
  const BleScannerScreen({super.key});
  @override
  State<BleScannerScreen> createState() => _BleScannerScreenState();
}

class _BleScannerScreenState extends State<BleScannerScreen> {
  List<ScanResult> _scanResults = [];
  bool _isScanning = false;

  @override
  void initState() {
    super.initState();
    FlutterBluePlus.scanResults.listen((results) => setState(() => _scanResults = results));
    FlutterBluePlus.isScanning.listen((state) => setState(() => _isScanning = state));
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: const Text("RGBW Remote")),
      body: ListView.builder(
        itemCount: _scanResults.length,
        itemBuilder: (context, index) {
          final res = _scanResults[index];
          if (res.device.platformName != "ESP32-C6-RGBW") return const SizedBox.shrink();
          return ListTile(
            title: Text(res.device.platformName),
            subtitle: Text(res.device.remoteId.str),
            trailing: ElevatedButton(
              child: const Text("Connect"),
              onPressed: () => Navigator.push(
                context, 
                MaterialPageRoute(builder: (_) => ControlScreen(device: res.device))
              ),
            ),
          );
        },
      ),
      floatingActionButton: FloatingActionButton(
        onPressed: () => FlutterBluePlus.startScan(timeout: const Duration(seconds: 4)),
        child: _isScanning ? const CircularProgressIndicator() : const Icon(Icons.search),
      ),
    );
  }
}

class ControlScreen extends StatefulWidget {
  final BluetoothDevice device;
  const ControlScreen({super.key, required this.device});
  @override
  State<ControlScreen> createState() => _ControlScreenState();
}

class _ControlScreenState extends State<ControlScreen> {
  BluetoothCharacteristic? targetChar;
  final List<double> _values = [0, 0, 0, 0]; // R, G, B, W
  bool _smoothFading = true; // Add tracking for the toggle

  @override
  void initState() {
    super.initState();
    _connect();
  }

  Future<void> _connect() async {
    await widget.device.connect(autoConnect: false, license: License.free);
    List<BluetoothService> services = await widget.device.discoverServices();
    for (var s in services) {
      for (var c in s.characteristics) {
        if (c.uuid.toString().contains("2a3d")) {
          setState(() => targetChar = c);
        }
      }
    }
  }

  // Extracted the sending logic so both sliders and the switch can trigger it
  void _sendData() {
    if (targetChar != null) {
      List<int> bytes = _values.map((e) => e.toInt()).toList();
      bytes.add(_smoothFading ? 1 : 0); // Append the 5th byte
      targetChar!.write(bytes, withoutResponse: true);
    }
  }

  void _updateColor(int index, double value) {
    setState(() => _values[index] = value);
    _sendData();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: const Text("Controller")),
      body: Column(
        children: [
          // The new UI Toggle for Smooth Fading
          SwitchListTile(
            title: const Text("Smooth Fading"),
            subtitle: const Text("Interpolate color changes"),
            value: _smoothFading,
            onChanged: (val) {
              setState(() => _smoothFading = val);
              _sendData(); // Instantly apply the setting
            },
          ),
          const Divider(),
          // The existing RGBW Sliders
          ...List.generate(4, (i) {
            final labels = ["Red", "Green", "Blue", "White"];
            final colors = [Colors.red, Colors.green, Colors.blue, Colors.grey];
            return Padding(
              padding: const EdgeInsets.symmetric(horizontal: 16.0, vertical: 8.0),
              child: Column(
                children: [
                  Text("${labels[i]}: ${_values[i].toInt()}"),
                  Slider(
                    value: _values[i],
                    min: 0, max: 255,
                    activeColor: colors[i],
                    onChanged: (v) => _updateColor(i, v),
                  ),
                ],
              ),
            );
          }),
        ],
      ),
    );
  }
}
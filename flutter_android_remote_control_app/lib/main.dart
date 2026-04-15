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
  final List<double> _values = [0, 0, 0, 0]; //R,G,B,W

  @override
  void initState() {
    super.initState();
    _connect();
  }

 Future<void> _connect() async {
    try {
      print("Attempting to connect...");
      await widget.device.connect(autoConnect: false, license: License.free);
      print("Connected! Discovering services...");
      
      List<BluetoothService> services = await widget.device.discoverServices();
      for (var s in services) {
        for (var c in s.characteristics) {
          print("Checking Characteristic: ${c.uuid}");
          // 16-bit UUIDs appear in Flutter as 0000xxxx-0000-1000-8000-00805f9b34fb
          if (c.uuid.toString().contains("2a3d")) {
            print("Target Characteristic Found!");
            setState(() => targetChar = c);
          }
        }
      }
    } catch (e) {
      print("Connection Error: $e");
    }
  }

  void _updateColor(int index, double value) {
    setState(() => _values[index] = value);
    if (targetChar != null) {
      // Convert our double values (0-255) to a byte list
      List<int> bytes = _values.map((e) => e.toInt()).toList();
      
      // We use withoutResponse: true for sliders to keep the UI fluid 
      // and avoid overwhelming the BLE stack with acknowledgments.
      targetChar!.write(bytes, withoutResponse: true);
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: const Text("Controller")),
      body: Column(
        children: List.generate(4, (i) {
          final labels = ["Red", "Green", "Blue", "White"];
          final colors = [Colors.red, Colors.green, Colors.blue, Colors.grey];
          return Padding(
            padding: const EdgeInsets.all(16.0),
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
      ),
    );
  }
}
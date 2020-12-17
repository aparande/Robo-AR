//
//  BLEViewController.swift
//  Robo-AR
//
//  Created by Anmol Parande on 11/15/20.
//  Copyright © 2020 Anmol Parande. All rights reserved.
//

import UIKit
import CoreBluetooth

/**
 An abstract view controller which implements BLE logic (so theoretically we could have different UI's which share the same BLE logic.
 Loosely based on https://www.raywenderlich.com/231-core-bluetooth-tutorial-for-ios-heart-rate-monitor
 */
class BLEViewController: UIViewController {
    // Reference to BLE manager
    var centralManager: CBCentralManager!
    // Reference to Romi Peripheral
    var romiPeripheral: CBPeripheral?
    
    // References to characteristics
    var instructionCharacteristic: CBCharacteristic?
    var acknowledgeCharacteristic: CBCharacteristic?
    
    // Reference to the status view
    @IBOutlet weak var bleStatusView: BLEStatusView?
    
    // Constants
    static let ROMI_NAME = "Robo-AR"
    static let ROMI_SERVICE_UUID = CBUUID(string: "4607EDA0-F65E-4D59-A9FF-84420D87A4CA")
    static let ROMI_INSTRUCTION_CHARACTERISTIC_UUID = CBUUID(string: "4607EDA1-F65E-4D59-A9FF-84420D87A4CA")
    static let ROMI_ACKNOWLEDGE_CHARACTERISTIC_UUID = CBUUID(string: "4607EDA2-F65E-4D59-A9FF-84420D87A4CA")
    static let ROMI_CHARACTERISTIC_UUIDS = [ROMI_INSTRUCTION_CHARACTERISTIC_UUID, ROMI_ACKNOWLEDGE_CHARACTERISTIC_UUID]
        
    override func viewDidLoad() {
        // Setup the BLE manager to start bluetooth
        centralManager = CBCentralManager(delegate: self, queue: nil)
    }
    
    /**
     Send an instruction via bluetooth
     */
    func sendInstruction(instruction: Instruction) {
        // Hide the instruction variable because parameters are immutable and we need a mutable paramter for UnsafeBufferPointers
        var instruction = instruction
        
        guard let romi = romiPeripheral, let characteristic = instructionCharacteristic else {
            print("Wasn't connected to Romi")
            return
        }
                        
        var payload = Data(buffer: UnsafeBufferPointer(start: &instruction.distance, count: 1))
        payload.append(Data(buffer: UnsafeBufferPointer(start: &instruction.angle, count: 1)))
        
        romi.writeValue(payload, for: characteristic, type: CBCharacteristicWriteType.withResponse)
        
        print("Transmitted instruction: (\(instruction)")
        bleStatusView?.instruction = instruction
        bleStatusView?.status = .transmitting
    }
    
    /**
     Template for compiling an instruction that subclasses must override
     */
    func compileNextInstruction(fromAck: Bool) -> Instruction? {
        preconditionFailure("Compile Next Instruction Not Implemented")
    }
    
    /**
     Reconnection logic
     */
    func onDisconnect() {
        // Try to reconnect if disconnected
        bleStatusView?.status = .connecting
        centralManager.scanForPeripherals(withServices: nil)
    }
}

/**
 Allow BLE View Controller to implement receive messags as the BLE Central
 */
extension BLEViewController: CBCentralManagerDelegate {
    /**
     System function called when BLE status changes
     */
    func centralManagerDidUpdateState(_ central: CBCentralManager) {
        switch central.state {
          case .unknown:
            print("central.state is .unknown")
          case .resetting:
            print("central.state is .resetting")
          case .unsupported:
            print("central.state is .unsupported")
          case .unauthorized:
            print("central.state is .unauthorized")
          case .poweredOff:
            print("central.state is .poweredOff")
          case .poweredOn:
            print("central.state is .poweredOn. Scanning for Romi")
            bleStatusView?.status = .connecting
            centralManager.scanForPeripherals(withServices: nil)
          default:
            print("Unrecognized state")
        }
    }
    
    /**
     System function for when a peripheral is discovered
     */
    func centralManager(_ central: CBCentralManager, didDiscover peripheral: CBPeripheral, advertisementData: [String : Any], rssi RSSI: NSNumber) {
        if peripheral.name == BLEViewController.ROMI_NAME {
            print(peripheral)
            romiPeripheral = peripheral
            romiPeripheral?.delegate = self
            centralManager.stopScan()
            centralManager.connect(peripheral, options: nil)
        }
    }
    
    /**
     System function for when the peripheral is connected to
     */
    func centralManager(_ central: CBCentralManager, didConnect peripheral: CBPeripheral) {
        print("Connected to Romi")
        bleStatusView?.status = .connected
        romiPeripheral?.discoverServices([BLEViewController.ROMI_SERVICE_UUID])
    }
}

/**
 Allow BLEViewController to receive messages from the peripheral
 */
extension BLEViewController: CBPeripheralDelegate {
    /**
     System function for when services are discovered on a connected peripheral
     */
    func peripheral(_ peripheral: CBPeripheral, didDiscoverServices error: Error?) {
        guard let services = peripheral.services else { return }
        for service in services {
            print(service)
            
            peripheral.discoverCharacteristics(BLEViewController.ROMI_CHARACTERISTIC_UUIDS, for: service)
        }
    }
    
    /**
     System function for when characteristics are discovered on a connected peripheral
     */
    func peripheral(_ peripheral: CBPeripheral, didDiscoverCharacteristicsFor service: CBService, error: Error?) {
        guard let characteristics = service.characteristics else { return }
        for characteristic in characteristics {
            if characteristic.uuid == BLEViewController.ROMI_INSTRUCTION_CHARACTERISTIC_UUID {
                print("Found instruction characteristic")
                instructionCharacteristic = characteristic
            } else if characteristic.uuid == BLEViewController.ROMI_ACKNOWLEDGE_CHARACTERISTIC_UUID {
                print("Found acknowledge characteristic")
                acknowledgeCharacteristic = characteristic
                
                // Enable notifications from the acknowledge characteristic
                romiPeripheral?.setNotifyValue(true, for: characteristic)
            }
        }
    }
    
    /**
     System function for reading values from characteristics on a connected peripheral
     */
    func peripheral(_ peripheral: CBPeripheral, didUpdateValueFor characteristic: CBCharacteristic, error: Error?) {
        switch characteristic.uuid {
        case BLEViewController.ROMI_INSTRUCTION_CHARACTERISTIC_UUID:
            print(characteristic.value ?? "No value for instruction")
        case BLEViewController.ROMI_ACKNOWLEDGE_CHARACTERISTIC_UUID:
            print(characteristic.value ?? "No value for acknowledge")
            guard let acknowledge = characteristic.value?.withUnsafeBytes({
                (pointer: UnsafePointer<Int>) -> Int in
                return pointer.pointee
            }) else { print("Couldn't get characteristic value"); return; }
            
            // If the Romi acknowledges, then try to send the next waypoint
            if acknowledge == 0 {
                if let inst = compileNextInstruction(fromAck: true) {
                    sendInstruction(instruction: inst)
                } else {
                    print("Finished executing instructions")
                    bleStatusView?.status = .done
                }
            } else {
                print("Romi Received Instruction!")
            }
        default:
            print("Unhandled characteristic UUID: \(characteristic.uuid)")
        }
    }
    
    /**
     System function for when a peripheral disconnects. Triggers reconnection logic
     */
    func centralManager(_ central: CBCentralManager, didDisconnectPeripheral peripheral: CBPeripheral, error: Error?) {
        print("Romi Disconnected")
        onDisconnect()
    }
}


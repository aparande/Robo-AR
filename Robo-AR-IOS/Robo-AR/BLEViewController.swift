//
//  BLEViewController.swift
//  Robo-AR
//
//  Created by Anmol Parande on 11/15/20.
//  Copyright © 2020 Anmol Parande. All rights reserved.
//

import UIKit
import CoreBluetooth

class BLEViewController: UIViewController {
    var instructions: [Instruction] = []
    var centralManager: CBCentralManager!
    var romiPeripheral: CBPeripheral?
    
    var instructionCharacteristic: CBCharacteristic?
    var acknowledgeCharacteristic: CBCharacteristic?
    
    static let ROMI_NAME = "Robo-AR"
    static let ROMI_SERVICE_UUID = CBUUID(string: "4607EDA0-F65E-4D59-A9FF-84420D87A4CA")
    static let ROMI_INSTRUCTION_CHARACTERISTIC_UUID = CBUUID(string: "4607EDA1-F65E-4D59-A9FF-84420D87A4CA")
    static let ROMI_ACKNOWLEDGE_CHARACTERISTIC_UUID = CBUUID(string: "4607EDA2-F65E-4D59-A9FF-84420D87A4CA")
    
    static let ROMI_CHARACTERISTIC_UUIDS = [ROMI_INSTRUCTION_CHARACTERISTIC_UUID, ROMI_ACKNOWLEDGE_CHARACTERISTIC_UUID]
    
    var lastExecutedInstruction = -1
    
    override func viewDidLoad() {
        centralManager = CBCentralManager(delegate: self, queue: nil)
    }
    
    func sendNextInstruction() {
        if lastExecutedInstruction + 1 >= instructions.count {
            print("Executed all instructions")
            return
        }
        
        guard let romi = romiPeripheral, let characteristic = instructionCharacteristic else {
            print("Wasn't connected to Romi")
            return
        }
        
        var instruction = instructions[lastExecutedInstruction + 1]
        var payload = Data(buffer: UnsafeBufferPointer(start: &instruction.distance, count: 1))
        payload.append(Data(buffer: UnsafeBufferPointer(start: &instruction.angle, count: 1)))
        
        romi.writeValue(payload, for: characteristic, type: CBCharacteristicWriteType.withResponse)
        
        lastExecutedInstruction += 1
        print("Transmitted instruction \(lastExecutedInstruction): (\(instructions[lastExecutedInstruction])")
    }
    
    func discoveredInstructionCharacteristic() {
        sendNextInstruction()
    }
}

extension BLEViewController: CBCentralManagerDelegate {
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
            centralManager.scanForPeripherals(withServices: nil)
        }
    }
    
    func centralManager(_ central: CBCentralManager, didDiscover peripheral: CBPeripheral, advertisementData: [String : Any], rssi RSSI: NSNumber) {
        if peripheral.name == BLEViewController.ROMI_NAME {
            print(peripheral)
            romiPeripheral = peripheral
            romiPeripheral?.delegate = self
            centralManager.stopScan()
            centralManager.connect(peripheral, options: nil)
        }
    }
    
    func centralManager(_ central: CBCentralManager, didConnect peripheral: CBPeripheral) {
        print("Connected to Romi")
        romiPeripheral?.discoverServices([BLEViewController.ROMI_SERVICE_UUID])
    }
}

extension BLEViewController: CBPeripheralDelegate {
    func peripheral(_ peripheral: CBPeripheral, didDiscoverServices error: Error?) {
        guard let services = peripheral.services else { return }
        for service in services {
            print(service)
            
            peripheral.discoverCharacteristics(BLEViewController.ROMI_CHARACTERISTIC_UUIDS, for: service)
        }
    }
    
    func peripheral(_ peripheral: CBPeripheral, didDiscoverCharacteristicsFor service: CBService, error: Error?) {
        guard let characteristics = service.characteristics else { return }
        for characteristic in characteristics {
            if characteristic.uuid == BLEViewController.ROMI_INSTRUCTION_CHARACTERISTIC_UUID {
                // Kick off the instruction sending process
                print("Found instruction characteristic")
                instructionCharacteristic = characteristic
                discoveredInstructionCharacteristic()
            } else if characteristic.uuid == BLEViewController.ROMI_ACKNOWLEDGE_CHARACTERISTIC_UUID {
                print("Found acknowledge characteristic")
                acknowledgeCharacteristic = characteristic
                romiPeripheral?.setNotifyValue(true, for: characteristic)
                print(acknowledgeCharacteristic?.properties.contains(.notify))
                romiPeripheral?.discoverDescriptors(for: characteristic)
            }
        }
    }
    
    func peripheral(_ peripheral: CBPeripheral, didUpdateValueFor characteristic: CBCharacteristic, error: Error?) {
        switch characteristic.uuid {
        case BLEViewController.ROMI_INSTRUCTION_CHARACTERISTIC_UUID:
            print(characteristic.value ?? "No value for instruction")
        case BLEViewController.ROMI_ACKNOWLEDGE_CHARACTERISTIC_UUID:
            print(characteristic.value ?? "No value for acknowledge")
        default:
            print("Unhandled characteristic UUID: \(characteristic.uuid)")
        }
    }
    
    func peripheral(_ peripheral: CBPeripheral, didUpdateNotificationStateFor characteristic: CBCharacteristic, error: Error?) {
        print("Updated characteristic notification \(characteristic) \(error)")
    }
    
    func peripheral(_ peripheral: CBPeripheral, didDiscoverDescriptorsFor characteristic: CBCharacteristic, error: Error?) {
        print(characteristic.descriptors)
    }
}


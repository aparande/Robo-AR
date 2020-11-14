//
//  InstructionTableViewController+CBCentralDelegate.swift
//  Robo-AR
//
//  Created by Anmol Parande on 11/13/20.
//  Copyright © 2020 Anmol Parande. All rights reserved.
//

import Foundation
import CoreBluetooth

extension InstructionTableViewController: CBCentralManagerDelegate {
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
        if peripheral.name == ROMI_NAME {
            print(peripheral)
            romiPeripheral = peripheral
            romiPeripheral.delegate = self
            centralManager.stopScan()
            centralManager.connect(romiPeripheral, options: nil)
        }
    }
    
    func centralManager(_ central: CBCentralManager, didConnect peripheral: CBPeripheral) {
        print("Connected to Romi")
        romiPeripheral.discoverServices([ROMI_SERVICE_UUID])
    }
}

extension InstructionTableViewController: CBPeripheralDelegate {
    func peripheral(_ peripheral: CBPeripheral, didDiscoverServices error: Error?) {
        guard let services = peripheral.services else { return }
        for service in services {
            print(service)
            
            peripheral.discoverCharacteristics([ROMI_INSTRUCTION_CHARACTERISTIC_UUID], for: service)
        }
    }
    
    func peripheral(_ peripheral: CBPeripheral, didDiscoverCharacteristicsFor service: CBService, error: Error?) {
        guard let characteristics = service.characteristics else { return }
        for characteristic in characteristics {
            if characteristic.uuid == ROMI_INSTRUCTION_CHARACTERISTIC_UUID {
                // Kick off the instruction sending process
                print("Found instruction characteristic")
                instructionCharacteristic = characteristic
                sendNextInstruction()
            }
        }
    }
    
    func peripheral(_ peripheral: CBPeripheral, didUpdateValueFor characteristic: CBCharacteristic, error: Error?) {
        switch characteristic.uuid {
        case ROMI_INSTRUCTION_CHARACTERISTIC_UUID:
            print(characteristic.value ?? "No Value")
        default:
            print("Unhandled characteristic UUID: \(characteristic.uuid)")
        }
    }
    
    func sendNextInstruction() {
        if lastExecutedInstruction + 1 >= instructions.count {
            print("Executed all instructions")
            return
        }
        
        var instruction = instructions[lastExecutedInstruction + 1]
        var payload = Data(buffer: UnsafeBufferPointer(start: &instruction.distance, count: 1))
        payload.append(Data(buffer: UnsafeBufferPointer(start: &instruction.angle, count: 1)))
        
        romiPeripheral.writeValue(payload, for: instructionCharacteristic, type: CBCharacteristicWriteType.withResponse)
        
        lastExecutedInstruction += 1
        print("Transmitted instruction \(lastExecutedInstruction)")
    }
}

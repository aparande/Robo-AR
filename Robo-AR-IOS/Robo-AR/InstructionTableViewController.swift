//
//  InstructionTableViewController.swift
//  Robo-AR
//
//  Created by Anmol Parande on 11/12/20.
//  Copyright © 2020 Anmol Parande. All rights reserved.
//

import UIKit
import CoreBluetooth

class InstructionTableViewController: UITableViewController {
    var instructions: [Instruction] = []
    var centralManager: CBCentralManager!
    var romiPeripheral: CBPeripheral!
    
    var instructionCharacteristic: CBCharacteristic!
    
    let ROMI_NAME = "DNEG"
    let ROMI_SERVICE_UUID = CBUUID(string: "4607EDA0-F65E-4D59-A9FF-84420D87A4CA")
    let ROMI_INSTRUCTION_CHARACTERISTIC_UUID = CBUUID(string: "4607EDA1-F65E-4D59-A9FF-84420D87A4CA")
    
    var lastExecutedInstruction = -1
    
    override func viewDidLoad() {
        centralManager = CBCentralManager(delegate: self, queue: nil)
    }
    
    
    // MARK: - Table view data source

    override func numberOfSections(in tableView: UITableView) -> Int {
        return 1
    }

    override func tableView(_ tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        return instructions.count
    }

    override func tableView(_ tableView: UITableView, cellForRowAt indexPath: IndexPath) -> UITableViewCell {
        let cell = tableView.dequeueReusableCell(withIdentifier: "instructionCell", for: indexPath)
        
        cell.textLabel?.text = "Distance: \(instructions[indexPath.row].distance)"
        cell.detailTextLabel?.text = "Angle:\(instructions[indexPath.row].angle * 180 / Float.pi)"
        
        if instructions[indexPath.row].completed {
            cell.accessoryType = .checkmark
        } else {
            cell.accessoryType = .none
        }
        
        return cell
    }
}

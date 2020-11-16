//
//  InstructionTableViewController.swift
//  Robo-AR
//
//  Created by Anmol Parande on 11/12/20.
//  Copyright © 2020 Anmol Parande. All rights reserved.
//

import UIKit
import CoreBluetooth

class InstructionTableViewController: BLEViewController, UITableViewDelegate, UITableViewDataSource {
    // MARK: - Table view data source

    func numberOfSections(in tableView: UITableView) -> Int {
        return 1
    }

    func tableView(_ tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        return instructions.count
    }

    func tableView(_ tableView: UITableView, cellForRowAt indexPath: IndexPath) -> UITableViewCell {
        let cell = tableView.dequeueReusableCell(withIdentifier: "instructionCell", for: indexPath)
        
        cell.textLabel?.text = "Distance: \(instructions[indexPath.row].distance)"
        cell.detailTextLabel?.text = "Angle:\(instructions[indexPath.row].angle)"
        
        if instructions[indexPath.row].completed {
            cell.accessoryType = .checkmark
        } else {    
            cell.accessoryType = .none
        }
        
        return cell
    }
}

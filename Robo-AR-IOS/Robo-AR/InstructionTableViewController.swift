//
//  InstructionTableViewController.swift
//  Robo-AR
//
//  Created by Anmol Parande on 11/12/20.
//  Copyright © 2020 Anmol Parande. All rights reserved.
//

import UIKit

class InstructionTableViewController: UITableViewController {
    var instructions: [Instruction] = []
    
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
        
        return cell
    }
}

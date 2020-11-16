//
//  ViewController.swift
//  Robo-AR
//
//  Created by Anmol Parande on 11/2/20.
//  Copyright © 2020 Anmol Parande. All rights reserved.
//

import UIKit
import RealityKit
import ARKit

enum Mode {
    case debug, release
}

class ViewController: BLEViewController {
    @IBOutlet var arView: WaypointView!
    @IBOutlet weak var generateButton: UIButton!
    
    let MODE = Mode.release
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        arView.addCoaching()
        arView.setupGestures()
        
        let config = ARWorldTrackingConfiguration()
        config.planeDetection = .horizontal
        arView.session.run(config, options: [.resetTracking, .removeExistingAnchors])
        
        arView.debugOptions.insert(.showSceneUnderstanding)
        arView.debugOptions.insert(.showWorldOrigin)
        //arView.debugOptions.insert(.showAnchorGeometry)
        
        generateButton.layer.cornerRadius = 20
    }
    
    @IBAction func startTransmission(_ sender: Any) {
        if MODE == .debug {
            performSegue(withIdentifier: "showInstructions", sender: nil)
        } else {
            for waypoint in arView.waypoints {
                guard let next = waypoint.next else { continue }
                instructions.append(Instruction(distance: waypoint.distanceTo(next), angle: waypoint.angleTo(next)))
            }
            
            sendNextInstruction()
        }
    }
    
    override func prepare(for segue: UIStoryboardSegue, sender: Any?) {
        guard let destination = segue.destination as? InstructionTableViewController else { return }

        for waypoint in arView.waypoints {
            guard let next = waypoint.next else { continue }
            destination.instructions.append(Instruction(distance: waypoint.distanceTo(next), angle: waypoint.angleTo(next)))
        }
        
    }
    
    override func discoveredInstructionCharacteristic() {
        // Do Nothing
    }
}
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
import CoreBluetooth


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
        
        guard let referenceImages = ARReferenceImage.referenceImages(inGroupNamed: "AR Resources", bundle: nil) else {
            fatalError("Missing expected asset catalog resources.")
        }
        
        let config = ARWorldTrackingConfiguration()
        config.detectionImages = referenceImages
        config.maximumNumberOfTrackedImages = 1
        config.planeDetection = .horizontal
        
        arView.session.delegate = self
        arView.session.run(config, options: [.resetTracking, .removeExistingAnchors])
        arView.debugOptions.insert(.showSceneUnderstanding)
        arView.debugOptions.insert(.showWorldOrigin)
        
        generateButton.layer.cornerRadius = 20
    }
    
    @IBAction func startTransmission(_ sender: Any) {
        if MODE == .debug {
            performSegue(withIdentifier: "showInstructions", sender: nil)
        } else {
            let inst = compileNextInstruction()
            if(inst != nil){
                sendNextInstruction(instruction: inst!)
            }
        }
    }
    
    override func prepare(for segue: UIStoryboardSegue, sender: Any?) {
        guard let destination = segue.destination as? InstructionTableViewController else { return }
        destination.instructions = instructions
    }
    
    override func compileNextInstruction() -> Instruction? {
        
        if(arView.robot == nil){
            return nil
        }
        if(arView.currentWayPoint == nil){
            arView.currentWayPoint = arView.waypoints.head
        }
        else {
            print("DISTANCE")
            print(arView.robot!.distanceTo(arView.currentWayPoint!))
            
            if(arView.robot!.distanceTo(arView.currentWayPoint!) < 0.10){
                arView.currentWayPoint = arView.currentWayPoint!.next
            }
        }
        
        if(arView.currentWayPoint == nil) {
            return nil
        }
        
        let waypoint = arView.currentWayPoint!
        let robot = arView.robot!
        
        print("ANGLE")
        print(robot.angleTo(waypoint))
        print("Distance")
        print(robot.distanceTo(waypoint))
        print("WAYPOINT NUMBER")
        print(waypoint.number)
        let instruction = Instruction(distance: robot.distanceTo(waypoint), angle: robot.angleTo(waypoint), waypointNumber: waypoint.number)
        
        instructions.append(instruction)
        
        return instruction
        
    }
    
    override func sendNextInstruction(instruction: Instruction) {
    
        guard let romi = romiPeripheral, let characteristic = instructionCharacteristic else {
            print("Wasn't connected to Romi")
            return
        }

        var instruction = instructions[instructions.count - 1]
        
        print(instruction.distance)
        print(instruction.angle)
                
        var payload = Data(buffer: UnsafeBufferPointer(start: &instruction.distance, count: 1))
        payload.append(Data(buffer: UnsafeBufferPointer(start: &instruction.angle, count: 1)))
        
        romi.writeValue(payload, for: characteristic, type: CBCharacteristicWriteType.withResponse)
        
        print("Transmitted instruction \(instructions.count - 1): (\(instruction)")
        bleStatusView?.instruction = instruction
        bleStatusView?.status = .transmitting
    }
    
    override func discoveredInstructionCharacteristic() {
        // Do Nothing
    }
}

extension ViewController: ARSessionDelegate {
    
    func session(_ session: ARSession, didAdd anchors: [ARAnchor]) {
        guard let imageAnchor = anchors.first as? ARImageAnchor,
              let _ = imageAnchor.referenceImage.name
        else { return }
        arView.addRobot(anchor: imageAnchor)
    }
    
    func session(_ session: ARSession, didUpdate anchors: [ARAnchor]) {
        guard let imageAnchor = anchors.first as? ARImageAnchor,
              let _ = imageAnchor.referenceImage.name
        else { return }
        arView.updateRobot(anchor: imageAnchor)
    }
}

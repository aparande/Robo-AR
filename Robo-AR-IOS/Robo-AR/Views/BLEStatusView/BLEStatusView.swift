//
//  StatusView.swift
//  Robo-AR
//
//  Created by Anmol Parande on 11/27/20.
//  Copyright © 2020 Anmol Parande. All rights reserved.
//

import UIKit

enum BLEStatus {
    case connected, connecting, transmitting
    case done
}

class BLEStatusView: UIView {
    @IBOutlet weak var loadingIndicator: UIActivityIndicatorView!
    @IBOutlet weak var statusLabel: UILabel!
    @IBOutlet var contentView: UIView!
    
    @IBOutlet weak var waypointLabel: UILabel!
    @IBOutlet weak var distLabel: UILabel!
    @IBOutlet weak var angleLabel: UILabel!
    
    private let statusGreen = UIColor(red: 39.0 / 255, green: 174.0 / 255, blue: 96.0 / 255, alpha: 1.0)
    private let statusRed = UIColor(red: 192.0 / 255, green: 57.0 / 255, blue: 43.0 / 255, alpha: 1.0)
    private let statusYellow = UIColor(red: 241.0 / 255, green: 196.0 / 255, blue: 15.0 / 255, alpha: 1.0)
    
    var instruction: Instruction? {
        didSet {
            if let inst = instruction {
                waypointLabel.text = "#: \(inst.waypointNumber)"
                distLabel.text = "Dist: \(String(format: "%.2f", inst.distance))"
                angleLabel.text = "Angle: \(String(format: "%.2f", inst.angle))"
            } else {
                waypointLabel.text = ""
                distLabel.text = ""
                angleLabel.text = ""
            }
        }
    }
    
    var status: BLEStatus = .connecting {
        didSet {
            switch status {
            case .connecting:
                statusLabel.text = "Connecting to Romi"
                contentView.backgroundColor = statusRed
                
                loadingIndicator.isHidden = false
                loadingIndicator.startAnimating()
            case .connected:
                statusLabel.text = "Connected to Romi"
                contentView.backgroundColor = statusGreen
                
                loadingIndicator.isHidden = true
                loadingIndicator.stopAnimating()
            case .transmitting:
                statusLabel.text = "Transmitting Instruction"
                contentView.backgroundColor = statusYellow
                
                loadingIndicator.isHidden = false
                loadingIndicator.startAnimating()
            case .done:
                statusLabel.text = "Transmitted all instructions"
                contentView.backgroundColor = statusGreen
                
                loadingIndicator.isHidden = true
                loadingIndicator.stopAnimating()
                
                instruction = nil
            }
        }
    }
    
    override init(frame: CGRect) {
        super.init(frame: frame)
        commonInit()
    }
    
    required init?(coder: NSCoder) {
        super.init(coder: coder)
        commonInit()
    }

    private func commonInit() {
        Bundle.main.loadNibNamed("BLEStatusView", owner: self, options: nil)
        addSubview(contentView)
        contentView.frame = self.bounds
        contentView.autoresizingMask = [.flexibleHeight, .flexibleWidth]
        
        instruction = nil
    }
}

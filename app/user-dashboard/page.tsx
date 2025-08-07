"use client"

import { useState, useEffect } from "react"
import { Card, CardContent, CardHeader, CardTitle } from "@/components/ui/card"
import { Button } from "@/components/ui/button"
import { Badge } from "@/components/ui/badge"
import {
  Zap,
  QrCode,
  CreditCard,
  Clock,
  Battery,
  Activity,
  AlertTriangle,
  CheckCircle,
  LogOut,
  Smartphone,
} from "lucide-react"
import { useRouter } from "next/navigation"

// Mock data for user's charging session
const userSession = {
  stationId: "ST003",
  stationName: "Downtown Mall - Station 3",
  status: "charging",
  startTime: "2:30 PM",
  duration: "45 min",
  batteryLevel: 78,
  chargingRate: "7.2 kW",
  voltage: "240V",
  current: "30A",
  cost: "$12.50",
  estimatedCompletion: "3:45 PM",
}

const chargingHistory = [
  {
    id: 1,
    date: "2024-01-15",
    station: "ST003",
    duration: "1h 20m",
    cost: "$18.75",
    energy: "25.4 kWh",
  },
  {
    id: 2,
    date: "2024-01-12",
    station: "ST001",
    duration: "55m",
    cost: "$14.20",
    energy: "18.8 kWh",
  },
  {
    id: 3,
    date: "2024-01-08",
    station: "ST004",
    duration: "2h 10m",
    cost: "$28.90",
    energy: "42.1 kWh",
  },
]

export default function UserDashboard() {
  const [isConnected, setIsConnected] = useState(false) // ESP32 connection status
  const [activeSession, setActiveSession] = useState(false)
  const [showQRScanner, setShowQRScanner] = useState(false)
  const router = useRouter()

  useEffect(() => {
    // Check if user is logged in and has proper role
    const userSession = localStorage.getItem("userSession")
    if (!userSession) {
      router.push("/login")
      return
    }

    const session = JSON.parse(userSession)
    if (session.role !== "end_user") {
      router.push("/") // Redirect admins to main dashboard
      return
    }

    // Simulate ESP32 connection check
    setIsConnected(false) // Set to false to show "No Data Available" state
    setActiveSession(false) // No active session when ESP32 not connected
  }, [router])

  const handleLogout = () => {
    localStorage.removeItem("userSession")
    sessionStorage.clear()
    router.push("/login")
  }

  const handleQRScan = () => {
    setShowQRScanner(true)
    // In real app, this would open camera for QR scanning
    setTimeout(() => {
      // Simulate QR code scan result
      const scannedStationId = "ST003"
      setShowQRScanner(false)
      handlePaymentFlow(scannedStationId)
    }, 2000)
  }

  const handlePaymentFlow = (stationId: string) => {
    // Simulate payment gateway redirect
    alert(`Redirecting to payment gateway for Station ${stationId}...`)

    // After successful payment, grant access
    setTimeout(() => {
      setActiveSession(true)
      setIsConnected(true)
      alert("Payment successful! Charging session started.")
    }, 1000)
  }

  return (
    <div className="min-h-screen bg-gradient-to-br from-slate-50 via-purple-50 to-green-50">
      {/* Header */}
      <header className="bg-white/80 backdrop-blur-xl border-b border-white/20 shadow-sm sticky top-0 z-40">
        <div className="flex items-center justify-between px-4 py-3">
          <div className="flex items-center space-x-2">
            <div className="w-8 h-8 bg-gradient-to-r from-green-400 to-purple-400 rounded-lg flex items-center justify-center">
              <Zap className="w-5 h-5 text-white" />
            </div>
            <span className="text-xl font-bold bg-gradient-to-r from-green-600 to-purple-600 bg-clip-text text-transparent">
              EV-Secure
            </span>
            <Badge variant="outline" className="ml-2 bg-blue-50 text-blue-700 border-blue-200">
              User Dashboard
            </Badge>
          </div>
          <Button
            variant="outline"
            onClick={handleLogout}
            className="bg-red-50 border-red-200 text-red-600 hover:bg-red-100"
          >
            <LogOut className="w-4 h-4 mr-2" />
            Logout
          </Button>
        </div>
      </header>

      <div className="p-4 sm:p-6 space-y-4 sm:space-y-6 max-w-4xl mx-auto">
        {/* Welcome Section */}
        <div className="text-center">
          <h1 className="text-2xl sm:text-3xl font-bold text-gray-800 mb-2">Welcome to EV-Secure</h1>
          <p className="text-gray-600">Scan QR codes at charging stations to start your session</p>
        </div>

        {/* QR Code Scanner */}
        <Card className="bg-white/80 backdrop-blur-xl border-white/20 shadow-lg">
          <CardHeader>
            <CardTitle className="text-gray-800 flex items-center gap-2">
              <QrCode className="w-5 h-5 text-purple-600" />
              Start Charging Session
            </CardTitle>
          </CardHeader>
          <CardContent>
            {!activeSession ? (
              <div className="text-center py-8">
                <div className="w-24 h-24 bg-gradient-to-r from-green-100 to-purple-100 rounded-full flex items-center justify-center mx-auto mb-4">
                  <QrCode className="w-12 h-12 text-purple-600" />
                </div>
                <h3 className="text-lg font-semibold text-gray-800 mb-2">Scan QR Code</h3>
                <p className="text-gray-600 mb-6">Find a charging station and scan the QR code to start your session</p>
                <Button
                  onClick={handleQRScan}
                  className="bg-gradient-to-r from-green-500 to-purple-500 text-white"
                  disabled={showQRScanner}
                >
                  {showQRScanner ? (
                    <div className="flex items-center space-x-2">
                      <div className="w-4 h-4 border-2 border-white border-t-transparent rounded-full animate-spin"></div>
                      <span>Scanning...</span>
                    </div>
                  ) : (
                    <>
                      <Smartphone className="w-4 h-4 mr-2" />
                      Open QR Scanner
                    </>
                  )}
                </Button>
              </div>
            ) : (
              <div className="bg-green-50 border border-green-200 rounded-lg p-4">
                <div className="flex items-center space-x-2 mb-2">
                  <CheckCircle className="w-5 h-5 text-green-600" />
                  <span className="font-semibold text-green-800">Session Active</span>
                </div>
                <p className="text-green-700">You have access to {userSession.stationName}</p>
              </div>
            )}
          </CardContent>
        </Card>

        {/* Current Session */}
        {activeSession && isConnected ? (
          <Card className="bg-white/80 backdrop-blur-xl border-white/20 shadow-lg">
            <CardHeader>
              <CardTitle className="text-gray-800 flex items-center gap-2">
                <Activity className="w-5 h-5 text-green-600" />
                Current Charging Session
              </CardTitle>
            </CardHeader>
            <CardContent>
              <div className="grid grid-cols-1 md:grid-cols-2 gap-6">
                <div className="space-y-4">
                  <div>
                    <h4 className="font-semibold text-gray-800 mb-2">Station Information</h4>
                    <p className="text-gray-600">{userSession.stationName}</p>
                    <Badge variant="outline" className="mt-1 bg-green-50 text-green-700 border-green-200">
                      {userSession.status}
                    </Badge>
                  </div>

                  <div>
                    <h4 className="font-semibold text-gray-800 mb-2">Session Details</h4>
                    <div className="space-y-2 text-sm">
                      <div className="flex justify-between">
                        <span className="text-gray-600">Start Time:</span>
                        <span className="font-medium">{userSession.startTime}</span>
                      </div>
                      <div className="flex justify-between">
                        <span className="text-gray-600">Duration:</span>
                        <span className="font-medium">{userSession.duration}</span>
                      </div>
                      <div className="flex justify-between">
                        <span className="text-gray-600">Cost:</span>
                        <span className="font-medium text-green-600">{userSession.cost}</span>
                      </div>
                    </div>
                  </div>
                </div>

                <div className="space-y-4">
                  <div>
                    <h4 className="font-semibold text-gray-800 mb-2">Battery Status</h4>
                    <div className="flex items-center space-x-3">
                      <Battery className="w-8 h-8 text-green-600" />
                      <div className="flex-1">
                        <div className="flex justify-between text-sm mb-1">
                          <span>Battery Level</span>
                          <span className="font-medium">{userSession.batteryLevel}%</span>
                        </div>
                        <div className="w-full bg-gray-200 rounded-full h-2">
                          <div
                            className="bg-green-500 h-2 rounded-full transition-all duration-300"
                            style={{ width: `${userSession.batteryLevel}%` }}
                          ></div>
                        </div>
                      </div>
                    </div>
                  </div>

                  <div>
                    <h4 className="font-semibold text-gray-800 mb-2">Live Metrics</h4>
                    <div className="grid grid-cols-2 gap-4 text-sm">
                      <div className="bg-blue-50 p-3 rounded-lg">
                        <p className="text-blue-600 font-medium">Charging Rate</p>
                        <p className="text-lg font-bold text-blue-800">{userSession.chargingRate}</p>
                      </div>
                      <div className="bg-purple-50 p-3 rounded-lg">
                        <p className="text-purple-600 font-medium">Voltage</p>
                        <p className="text-lg font-bold text-purple-800">{userSession.voltage}</p>
                      </div>
                      <div className="bg-orange-50 p-3 rounded-lg">
                        <p className="text-orange-600 font-medium">Current</p>
                        <p className="text-lg font-bold text-orange-800">{userSession.current}</p>
                      </div>
                      <div className="bg-green-50 p-3 rounded-lg">
                        <p className="text-green-600 font-medium">Est. Complete</p>
                        <p className="text-lg font-bold text-green-800">{userSession.estimatedCompletion}</p>
                      </div>
                    </div>
                  </div>
                </div>
              </div>

              <div className="mt-6 flex justify-center">
                <Button
                  variant="outline"
                  onClick={() => {
                    setActiveSession(false)
                    setIsConnected(false)
                    alert("Charging session ended.")
                  }}
                  className="bg-red-50 border-red-200 text-red-600 hover:bg-red-100"
                >
                  End Session
                </Button>
              </div>
            </CardContent>
          </Card>
        ) : activeSession && !isConnected ? (
          <Card className="bg-white/80 backdrop-blur-xl border-white/20 shadow-lg">
            <CardHeader>
              <CardTitle className="text-gray-800 flex items-center gap-2">
                <AlertTriangle className="w-5 h-5 text-orange-600" />
                Session Status
              </CardTitle>
            </CardHeader>
            <CardContent>
              <div className="text-center py-8">
                <AlertTriangle className="w-16 h-16 text-orange-500 mx-auto mb-4" />
                <h3 className="text-lg font-semibold text-gray-800 mb-2">No Data Available</h3>
                <p className="text-gray-600 mb-4">
                  ESP32-S3 station is not connected. Unable to retrieve live charging data.
                </p>
                <Badge variant="outline" className="bg-orange-50 text-orange-700 border-orange-200">
                  ESP32-S3 Disconnected
                </Badge>
              </div>
            </CardContent>
          </Card>
        ) : null}

        {/* Charging History */}
        <Card className="bg-white/80 backdrop-blur-xl border-white/20 shadow-lg">
          <CardHeader>
            <CardTitle className="text-gray-800 flex items-center gap-2">
              <Clock className="w-5 h-5 text-blue-600" />
              Charging History
            </CardTitle>
          </CardHeader>
          <CardContent>
            <div className="space-y-4">
              {chargingHistory.map((session) => (
                <div key={session.id} className="flex items-center justify-between p-4 bg-gray-50 rounded-lg">
                  <div>
                    <p className="font-medium text-gray-800">{session.station}</p>
                    <p className="text-sm text-gray-600">{session.date}</p>
                  </div>
                  <div className="text-right">
                    <p className="font-medium text-gray-800">{session.cost}</p>
                    <p className="text-sm text-gray-600">
                      {session.duration} • {session.energy}
                    </p>
                  </div>
                </div>
              ))}
            </div>
          </CardContent>
        </Card>

        {/* Payment Methods */}
        <Card className="bg-white/80 backdrop-blur-xl border-white/20 shadow-lg">
          <CardHeader>
            <CardTitle className="text-gray-800 flex items-center gap-2">
              <CreditCard className="w-5 h-5 text-green-600" />
              Payment Methods
            </CardTitle>
          </CardHeader>
          <CardContent>
            <div className="text-center py-6">
              <CreditCard className="w-12 h-12 text-gray-400 mx-auto mb-4" />
              <h3 className="text-lg font-semibold text-gray-800 mb-2">Secure Payment</h3>
              <p className="text-gray-600 mb-4">
                Payment is processed securely when you scan QR codes at charging stations
              </p>
              <Badge variant="outline" className="bg-green-50 text-green-700 border-green-200">
                SSL Encrypted
              </Badge>
            </div>
          </CardContent>
        </Card>

        {/* Access Limitations Notice */}
        <div className="bg-blue-50 border border-blue-200 rounded-lg p-4">
          <div className="flex items-start space-x-3">
            <AlertTriangle className="w-5 h-5 text-blue-600 flex-shrink-0 mt-0.5" />
            <div>
              <h4 className="font-semibold text-blue-800 mb-1">Limited Access Account</h4>
              <p className="text-sm text-blue-700">
                As an end user, you can only access charging stations through QR code scanning and payment. You cannot
                view other stations' data or access administrative functions.
              </p>
            </div>
          </div>
        </div>
      </div>
    </div>
  )
}

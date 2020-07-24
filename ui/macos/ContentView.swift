import SwiftUI
import WebKit

struct WebView: NSViewRepresentable {
    
    var content: String

    func makeNSView(context: NSViewRepresentableContext<WebView>) -> WKWebView {
        let webView = WKWebView()
        
        webView.loadHTMLString(content, baseURL: Bundle.main.bundleURL)
        webView.allowsBackForwardNavigationGestures = false

        return webView
    }

    func updateNSView(_ uiView: WKWebView, context: NSViewRepresentableContext<WebView>) {

    }
}

struct ContentView: View {
    var webView: WKWebView!
    var body: some View {
        VStack() {
            WebView(content: "<!DOCTYPE html><html><body><svg height='210' width='500'><polygon points='100,10 40,198 190,78 10,78 160,198' style='fill:lime;stroke:purple;stroke-width:5;fill-rule:evenodd;'/></svg></body></html>")
            Text("GStreamer Instruments")
        }
    }
}


struct ContentView_Previews: PreviewProvider {
    static var previews: some View {
        ContentView()
    }
}

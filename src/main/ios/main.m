// A helper file used to run benchmark in iOS.

#import <UIKit/UIKit.h>
#import <WebKit/WebKit.h>

/// benchmark entrance
extern void benchmark(const char *path);

/// benchmark data path provider
const char *benchmark_get_data_path(void) {
    return [[NSBundle mainBundle] bundlePath].UTF8String;
}


@interface WebViewController : UIViewController <UIDocumentInteractionControllerDelegate>
@property (nonatomic, strong) WKWebView *webView;
@property (nonatomic, strong) NSURL *fileURL;
@property (nonatomic, strong) UIDocumentInteractionController *shareMenu;
@end

@implementation WebViewController
- (instancetype)initWithFileURL:(NSURL *)fileURL {
    self = [self init];
    self.fileURL = fileURL;
    self.webView = [WKWebView new];
    [self.webView loadFileURL:fileURL allowingReadAccessToURL:fileURL];
    
    UIBarButtonItem *barItem = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemAction target:self action:@selector(shareFile)];
    self.navigationItem.rightBarButtonItem = barItem;
    return self;
}

- (void)loadView {
    [super loadView];
    [self.view addSubview:self.webView];
}

- (void)viewDidLayoutSubviews {
    [super viewDidLayoutSubviews];
    self.webView.frame = self.view.frame;
}

- (void)shareFile {
    if (_shareMenu) return;
    
    _shareMenu = [UIDocumentInteractionController interactionControllerWithURL:_fileURL];
    _shareMenu.delegate = self;
    _shareMenu.UTI = @"public.html";
    
    UIViewController *nav = self.navigationController;
    [_shareMenu presentOpenInMenuFromRect:nav.view.bounds inView:nav.view animated:YES];
}

#pragma mark - Document

- (UIViewController *)documentInteractionControllerViewControllerForPreview:(UIDocumentInteractionController *)controller {
    return self.navigationController;
}

- (CGRect)documentInteractionControllerRectForPreview:(UIDocumentInteractionController *)controller {
    return self.navigationController.view.bounds;
}

- (nullable UIView *)documentInteractionControllerViewForPreview:(UIDocumentInteractionController *)controller {
    return self.navigationController.view;
}

- (void)documentInteractionControllerDidDismissOpenInMenu:(UIDocumentInteractionController *)controller {
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0.5 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        self.shareMenu = nil;
    });
}

@end



@interface MainController : UITableViewController
@property (nonatomic, strong) NSMutableArray<NSString *> *names;
@property (nonatomic, assign) int lastNum;
@end

@implementation MainController

- (void)viewDidLoad {
    self.title = @"benchmark";
    [self setBarItemRunning:NO];
    
    [self.tableView registerClass:UITableViewCell.class forCellReuseIdentifier:@"cell"];
    
    self.names = [NSMutableArray new];
    NSString *dir = [self documentsPath];
    NSArray<NSString *> *items = [[NSFileManager defaultManager] contentsOfDirectoryAtPath:dir error:nil];
    items = [items sortedArrayUsingSelector:@selector(compare:)];
    for (NSString *item in items) {
        if ([item hasPrefix:@"report"] && [item.pathExtension isEqualToString:@"html"]) {
            [self.names addObject:item];
            NSString *numStr = [item substringWithRange:NSMakeRange(6, item.length - 11)];
            int num = numStr.intValue;
            if (num > _lastNum) _lastNum = num;
        }
    }
    [self.tableView reloadData];
}

- (void)run {
    [self setBarItemRunning:YES];
    
    NSString *name = [NSString stringWithFormat:@"report%d.html", _lastNum + 1];
    NSString *path = [[self documentsPath] stringByAppendingPathComponent:name];
    
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_HIGH, 0), ^{
        
        benchmark(path.UTF8String);
        
        if (![[NSFileManager defaultManager] fileExistsAtPath:path]) return;
        dispatch_async(dispatch_get_main_queue(), ^{
            [self setBarItemRunning:NO];
            [self.names addObject:name];
            self.lastNum++;
            
            NSIndexPath *indexPath = [NSIndexPath indexPathForRow:self.names.count - 1 inSection:0];
            [self.tableView insertRowsAtIndexPaths:@[indexPath] withRowAnimation:UITableViewRowAnimationAutomatic];
            [self.tableView scrollToRowAtIndexPath:indexPath atScrollPosition:UITableViewScrollPositionNone animated:YES];
        });
    });
}

- (void)setBarItemRunning:(BOOL)runnning {
    UIBarButtonItem *barItem;
    if (runnning) {
        UIActivityIndicatorViewStyle style;
        if (@available(iOS 13, *)) {
            style = UIActivityIndicatorViewStyleMedium;
        } else {
            style = 2; // UIActivityIndicatorViewStyleGray;
        }
        UIActivityIndicatorView *hud = [[UIActivityIndicatorView alloc] initWithActivityIndicatorStyle:style];
        [hud startAnimating];
        barItem = [[UIBarButtonItem alloc] initWithCustomView:hud];
    } else {
        barItem = [[UIBarButtonItem alloc] initWithTitle:@"run" style:UIBarButtonItemStylePlain target:self action:@selector(run)];
    }
    self.navigationItem.rightBarButtonItem = barItem;
}

- (NSString *)documentsPath {
    return [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) firstObject];
}

#pragma mark TableView

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
    return _names.count;
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    UITableViewCell *cell = [self.tableView dequeueReusableCellWithIdentifier:@"cell" forIndexPath:indexPath];
    cell.textLabel.text = _names[indexPath.row];
    return cell;
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
    [self.tableView deselectRowAtIndexPath:indexPath animated:YES];
    
    NSString *name = _names[indexPath.row];
    NSString *path = [[self documentsPath] stringByAppendingPathComponent:name];
    NSURL *url = [NSURL fileURLWithPath:path];
    WebViewController *vc = [[WebViewController alloc] initWithFileURL:url];
    [self.navigationController pushViewController:vc animated:YES];
}

- (void)tableView:(UITableView *)tableView commitEditingStyle:(UITableViewCellEditingStyle)editingStyle forRowAtIndexPath:(NSIndexPath *)indexPath {
    NSString *path = [[self documentsPath] stringByAppendingPathComponent:self.names[indexPath.row]];
    [[NSFileManager defaultManager] removeItemAtPath:path error:nil];
    [self.names removeObjectAtIndex:indexPath.row];
    [self.tableView deleteRowsAtIndexPaths:@[indexPath] withRowAnimation:UITableViewRowAnimationAutomatic];
}

@end



@interface AppDelegate : UIResponder <UIApplicationDelegate>
@property (nonatomic, strong) UIWindow *window;
@property (nonatomic, strong) UINavigationController *nav;
@end

@implementation AppDelegate

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    MainController *root = [MainController new];
    UINavigationController *nav = [[UINavigationController alloc] initWithRootViewController:root];
    self.nav = nav;
    self.window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
    self.window.rootViewController = self.nav;
    self.window.backgroundColor = [UIColor grayColor];
    [self.window makeKeyAndVisible];
    return YES;
}

@end

int main(int argc, char * argv[]) {
    @autoreleasepool {
        return UIApplicationMain(argc, argv, nil, NSStringFromClass([AppDelegate class]));
    }
}
